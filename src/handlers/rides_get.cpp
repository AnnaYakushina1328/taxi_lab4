#include "rides_get.hpp"

#include <chrono>
#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../performance/simple_performance.hpp"

namespace taxi {

namespace {

constexpr int kActiveRidesLimitPerMinute = 100;

userver::formats::json::Value MakeRideJson(const Ride& ride) {
  userver::formats::json::ValueBuilder item;
  item["id"] = ride.id;
  item["passenger_id"] = ride.passenger_id;
  item["driver_id"] = ride.driver_id;
  item["pickup_address"] = ride.pickup_address;
  item["destination_address"] = ride.destination_address;
  item["status"] = ride.status;
  return item.ExtractValue();
}

void SetRateLimitHeaders(
    userver::server::http::HttpResponse& response,
    const performance::RateLimitResult& result
) {
  response.SetHeader(
      std::string{"X-RateLimit-Limit"},
      std::to_string(result.limit)
  );

  response.SetHeader(
      std::string{"X-RateLimit-Remaining"},
      std::to_string(result.remaining)
  );

  response.SetHeader(
      std::string{"X-RateLimit-Reset"},
      std::to_string(result.reset_epoch_seconds)
  );
}

}  // namespace

RidesGet::RidesGet(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<TaxiStorageComponent>().GetStorage()) {
}

std::string RidesGet::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto user_id_arg = request.GetArg("user_id");
  const auto status = request.GetArg("status");

  const bool is_history_query = !user_id_arg.empty();
  const bool is_active_query = user_id_arg.empty() && status == "active";

  if (!is_history_query && !is_active_query) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder error;
    error["error"] = "user_id or status=active query parameter is required";
    return userver::formats::json::ToString(error.ExtractValue());
  }

  if (is_active_query) {
    const auto limit_result = performance::GetRateLimiter().Check(
        "GET:/rides?status=active",
        kActiveRidesLimitPerMinute,
        std::chrono::minutes{1}
    );

    SetRateLimitHeaders(response, limit_result);

    if (!limit_result.allowed) {
      response.SetHeader(std::string{"Retry-After"}, std::string{"60"});
      response.SetStatus(userver::server::http::HttpStatus::kTooManyRequests);

      userver::formats::json::ValueBuilder error;
      error["error"] = "too many requests";
      return userver::formats::json::ToString(error.ExtractValue());
    }
  }

  const std::string cache_key =
      is_history_query ? "rides:history:" + user_id_arg : "rides:active";

  if (const auto cached = performance::GetCache().Get(cache_key)) {
    response.SetHeader(std::string{"X-Cache"}, std::string{"HIT"});
    return *cached;
  }

  response.SetHeader(std::string{"X-Cache"}, std::string{"MISS"});

  userver::formats::json::ValueBuilder result(userver::formats::common::Type::kArray);

  if (is_history_query) {
    const int user_id = std::stoi(user_id_arg);
    const auto rides = storage_.GetRidesByUserId(user_id);

    for (const auto& ride : rides) {
      result.PushBack(MakeRideJson(ride));
    }

    const auto body = userver::formats::json::ToString(result.ExtractValue());
    performance::GetCache().Set(cache_key, body, std::chrono::seconds{60});

    return body;
  }

  const auto rides = storage_.GetActiveRides();

  for (const auto& ride : rides) {
    result.PushBack(MakeRideJson(ride));
  }

  const auto body = userver::formats::json::ToString(result.ExtractValue());
  performance::GetCache().Set(cache_key, body, std::chrono::seconds{15});

  return body;
}

userver::yaml_config::Schema RidesGet::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<
      userver::server::handlers::HttpHandlerBase>(R"(
type: object
description: get rides handler
additionalProperties: false
properties: {}
)");
}

}  // namespace taxi
