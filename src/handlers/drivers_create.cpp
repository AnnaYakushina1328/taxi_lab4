#include "drivers_create.hpp"

#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

DriversCreate::DriversCreate(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string DriversCreate::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto json = userver::formats::json::FromString(request.RequestBody());

  const auto user_id = json["user_id"].As<int>();
  const auto car_model = json["car_model"].As<std::string>();
  const auto car_number = json["car_number"].As<std::string>();
  const auto license_number = json["license_number"].As<std::string>();

  if (user_id <= 0 || car_model.empty() || car_number.empty() ||
      license_number.empty()) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] =
        "user_id, car_model, car_number and license_number are required";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  const auto driver =
      storage_.CreateDriver(user_id, car_model, car_number, license_number);
  if (!driver) {
    response.SetStatus(userver::server::http::HttpStatus::kConflict);
    userver::formats::json::ValueBuilder error;
    error["error"] = "driver cannot be created";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  performance::GetCache().InvalidatePrefix("drivers:");

  response.SetStatus(userver::server::http::HttpStatus::kCreated);

  userver::formats::json::ValueBuilder result;
  result["id"] = driver->id;
  result["user_id"] = driver->user_id;
  result["car_model"] = driver->car_model;
  result["car_number"] = driver->car_number;
  result["license_number"] = driver->license_number;
  result["status"] = driver->status;

  return userver::formats::json::ToString(result.ExtractValue());
}

userver::yaml_config::Schema DriversCreate::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: create driver handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
