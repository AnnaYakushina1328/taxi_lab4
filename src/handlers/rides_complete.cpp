#include "rides_complete.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

RidesComplete::RidesComplete(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string RidesComplete::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto ride_id_arg = request.GetPathArg("id");
  if (ride_id_arg.empty()) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] = "ride id is required in path";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  const int ride_id = std::stoi(ride_id_arg);

  const auto ride = storage_.CompleteRide(ride_id);
  if (!ride) {
    response.SetStatus(userver::server::http::HttpStatus::kConflict);
    userver::formats::json::ValueBuilder error;
    error["error"] = "ride cannot be completed";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  performance::GetCache().InvalidatePrefix("rides:");

  userver::formats::json::ValueBuilder result;
  result["id"] = ride->id;
  result["passenger_id"] = ride->passenger_id;
  result["driver_id"] = ride->driver_id;
  result["pickup_address"] = ride->pickup_address;
  result["destination_address"] = ride->destination_address;
  result["status"] = ride->status;

  return userver::formats::json::ToString(result.ExtractValue());
}

userver::yaml_config::Schema RidesComplete::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: complete ride handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
