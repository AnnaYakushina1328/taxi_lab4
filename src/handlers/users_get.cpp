#include "users_get.hpp"

#include <chrono>
#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

UsersGet::UsersGet(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string UsersGet::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto login = request.GetArg("login");
  const auto name_mask = request.GetArg("name_mask");

  const bool has_login = !login.empty();
  const bool has_name_mask = !name_mask.empty();

  if (has_login == has_name_mask) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] = "use exactly one query parameter: login or name_mask";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  const std::string cache_key =
      has_login ? "users:login:" + login : "users:name_mask:" + name_mask;

  if (const auto cached = performance::GetCache().Get(cache_key)) {
    response.SetHeader(std::string{"X-Cache"}, std::string{"HIT"});
    return *cached;
  }

  response.SetHeader(std::string{"X-Cache"}, std::string{"MISS"});

  if (has_login) {
    const auto user = storage_.GetUserByLogin(login);
    if (!user) {
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      userver::formats::json::ValueBuilder error;
      error["error"] = "user not found";
      return userver::formats::json::ToString(error.ExtractValue());
    }

    userver::formats::json::ValueBuilder result;
    result["id"] = user->id;
    result["login"] = user->login;
    result["full_name"] = user->full_name;

    const auto body = userver::formats::json::ToString(result.ExtractValue());
    performance::GetCache().Set(cache_key, body, std::chrono::seconds{60});

    return body;
  }

  const auto users = storage_.FindUsersByNameMask(name_mask);

  userver::formats::json::ValueBuilder result(userver::formats::common::Type::kArray);
  for (const auto& user : users) {
    userver::formats::json::ValueBuilder item;
    item["id"] = user.id;
    item["login"] = user.login;
    item["full_name"] = user.full_name;
    result.PushBack(item.ExtractValue());
  }

  const auto body = userver::formats::json::ToString(result.ExtractValue());
  performance::GetCache().Set(cache_key, body, std::chrono::seconds{60});

  return body;
}

userver::yaml_config::Schema UsersGet::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: get users handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
