#include "auth_middleware.hpp"

#include <string>

#include <userver/server/http/http_status.hpp>

namespace taxi {

namespace {

bool StartsWith(const std::string& value, const std::string& prefix) {
  return value.rfind(prefix, 0) == 0;
}

}  // namespace

AuthMiddleware::AuthMiddleware(
    const userver::server::handlers::HttpHandlerBase&,
    TaxiStorage& storage)
    : storage_(storage) {
}

bool AuthMiddleware::IsPublicRequest(
    const userver::server::http::HttpRequest& request) const {
  const auto& path = request.GetRequestPath();

  if (path == "/ping") return true;
  if (path == "/users") return true;
  if (path == "/auth/login") return true;
  if (path == "/mongo/users") return true;
  if (StartsWith(path, "/tests/")) return true;

  return false;
}

void AuthMiddleware::HandleRequest(
    userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
  if (IsPublicRequest(request)) {
    Next(request, context);
    return;
  }

  const auto& auth_header = request.GetHeader("Authorization");
  constexpr std::string_view kBearerPrefix = "Bearer ";

  if (auth_header.size() <= kBearerPrefix.size() ||
      auth_header.compare(0, kBearerPrefix.size(), kBearerPrefix) != 0) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
    response.SetContentType("application/json");
    response.SetData(R"({"error":"missing or invalid Authorization header"})");
    return;
  }

  const std::string token = auth_header.substr(kBearerPrefix.size());
  if (!storage_.IsTokenValid(token)) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
    response.SetContentType("application/json");
    response.SetData(R"({"error":"invalid token"})");
    return;
  }

  Next(request, context);
}

AuthMiddlewareFactory::AuthMiddlewareFactory(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpMiddlewareFactoryBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase>
AuthMiddlewareFactory::Create(
    const userver::server::handlers::HttpHandlerBase& handler,
    userver::yaml_config::YamlConfig) const {
  return std::make_unique<AuthMiddleware>(handler, storage_);
}

}  // namespace taxi
