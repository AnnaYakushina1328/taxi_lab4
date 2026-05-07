#include "rides_create.hpp"

#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

RidesCreate::RidesCreate(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string RidesCreate::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto json = userver::formats::json::FromString(request.RequestBody());

  const auto passenger_id = json["passenger_id"].As<int>();
  const auto pickup_address = json["pickup_address"].As<std::string>();
  const auto destination_address = json["destination_address"].As<std::string>();

  if (passenger_id <= 0 || pickup_address.empty() || destination_address.empty()) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] =
        "passenger_id, pickup_address and destination_address are required";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  const auto ride =
      storage_.CreateRide(passenger_id, pickup_address, destination_address);
  if (!ride) {
    response.SetStatus(userver::server::http::HttpStatus::kConflict);
    userver::formats::json::ValueBuilder error;
    error["error"] = "ride cannot be created";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  performance::GetCache().InvalidatePrefix("rides:");

  response.SetStatus(userver::server::http::HttpStatus::kCreated);

  userver::formats::json::ValueBuilder result;
  result["id"] = ride->id;
  result["passenger_id"] = ride->passenger_id;
  result["driver_id"] = ride->driver_id;
  result["pickup_address"] = ride->pickup_address;
  result["destination_address"] = ride->destination_address;
  result["status"] = ride->status;

  return userver::formats::json::ToString(result.ExtractValue());
}

userver::yaml_config::Schema RidesCreate::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: create ride handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
