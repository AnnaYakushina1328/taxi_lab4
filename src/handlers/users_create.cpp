#include "users_create.hpp"

#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

UsersCreate::UsersCreate(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string UsersCreate::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto json = userver::formats::json::FromString(request.RequestBody());

  const auto login = json["login"].As<std::string>();
  const auto password = json["password"].As<std::string>();
  const auto full_name = json["full_name"].As<std::string>();

  if (login.empty() || password.empty() || full_name.empty()) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] = "login, password and full_name are required";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  const auto user = storage_.CreateUser(login, password, full_name);
  if (!user) {
    response.SetStatus(userver::server::http::HttpStatus::kConflict);
    userver::formats::json::ValueBuilder error;
    error["error"] = "user already exists";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  performance::GetCache().InvalidatePrefix("users:");

  response.SetStatus(userver::server::http::HttpStatus::kCreated);

  userver::formats::json::ValueBuilder result;
  result["id"] = user->id;
  result["login"] = user->login;
  result["full_name"] = user->full_name;

  return userver::formats::json::ToString(result.ExtractValue());
}

userver::yaml_config::Schema UsersCreate::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: create user handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
