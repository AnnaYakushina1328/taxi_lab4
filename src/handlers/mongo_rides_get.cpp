#include "mongo_rides_get.hpp"

#include <string>

#include <userver/formats/bson.hpp>
#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>
#include <userver/storages/mongo/component.hpp>

namespace taxi {
namespace {

userver::formats::json::Value MakeRideJson(const userver::formats::bson::Value& doc) {
  using Oid = userver::formats::bson::Oid;

  userver::formats::json::ValueBuilder builder;

  if (doc.HasMember("_id")) {
    builder["id"] = doc["_id"].As<Oid>().ToString();
  }
  if (doc.HasMember("status")) {
    builder["status"] = doc["status"].As<std::string>();
  }

  if (doc.HasMember("route")) {
    const auto route = doc["route"];
    if (route.HasMember("pickup_address")) {
      builder["pickup_address"] = route["pickup_address"].As<std::string>();
    }
    if (route.HasMember("dropoff_address")) {
      builder["destination_address"] = route["dropoff_address"].As<std::string>();
    }
  }

  if (doc.HasMember("fare")) {
    const auto fare = doc["fare"];
    if (fare.HasMember("amount")) {
      builder["amount"] = fare["amount"].As<int>();
    }
    if (fare.HasMember("currency")) {
      builder["currency"] = fare["currency"].As<std::string>();
    }
  }

  if (doc.HasMember("user_snapshot")) {
    const auto user_snapshot = doc["user_snapshot"];
    if (user_snapshot.HasMember("login")) {
      builder["user_login"] = user_snapshot["login"].As<std::string>();
    }
  }

  try {
    if (doc.HasMember("driver_snapshot")) {
      const auto driver_snapshot = doc["driver_snapshot"];
      if (driver_snapshot.HasMember("login")) {
        builder["driver_login"] = driver_snapshot["login"].As<std::string>();
      }
    }
  } catch (const std::exception&) {
  }

  builder["source"] = "mongo";
  return builder.ExtractValue();
}

}  // namespace

MongoRidesGet::MongoRidesGet(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

std::string MongoRidesGet::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  const auto user_login = request.GetArg("user_login");
  const auto status = request.GetArg("status");

  const bool by_user = !user_login.empty();
  const bool by_status = !status.empty();

  if (by_user == by_status) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"use exactly one query parameter: user_login or status"})";
  }

  try {
    using userver::formats::bson::MakeArray;
    using userver::formats::bson::MakeDoc;

    auto rides = pool_->GetCollection("rides");

    userver::formats::bson::Document query;

    if (by_status) {
      if (status != "active") {
        response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error":"only status=active is supported"})";
      }
      query = MakeDoc("status", MakeDoc("$in", MakeArray("created", "accepted")));
    } else {
      query = MakeDoc("user_snapshot.login", user_login);
    }

    auto cursor = rides.Find(query);

    userver::formats::json::ValueBuilder result(userver::formats::common::Type::kArray);
    for (const auto& doc : cursor) {
      result.PushBack(MakeRideJson(doc));
    }

    response.SetStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(result.ExtractValue());
  } catch (const std::exception&) {
    response.SetStatus(userver::server::http::HttpStatus::kInternalServerError);
    return R"({"error":"failed to read mongo rides"})";
  }
}

userver::yaml_config::Schema MongoRidesGet::GetStaticConfigSchema() {
  return userver::server::handlers::HttpHandlerBase::GetStaticConfigSchema();
}

}  // namespace taxi
