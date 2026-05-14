#include "mongo_rides_create.hpp"

#include <chrono>
#include <string>

#include <userver/formats/bson.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>
#include <userver/storages/mongo/component.hpp>

namespace taxi {
namespace {

userver::formats::json::Value MakeRideResponse(
    const std::string& id,
    const std::string& user_login,
    const std::string& pickup_address,
    const std::string& destination_address,
    int amount,
    const std::string& status) {
  userver::formats::json::ValueBuilder result;
  result["id"] = id;
  result["user_login"] = user_login;
  result["pickup_address"] = pickup_address;
  result["destination_address"] = destination_address;
  result["amount"] = amount;
  result["currency"] = "RUB";
  result["status"] = status;
  result["source"] = "mongo";
  return result.ExtractValue();
}

}  // namespace

MongoRidesCreate::MongoRidesCreate(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

std::string MongoRidesCreate::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto& response = request.GetHttpResponse();
  response.SetContentType("application/json");

  userver::formats::json::Value body;
  try {
    body = userver::formats::json::FromString(request.RequestBody());
  } catch (const std::exception&) {
    response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    return R"({"error":"invalid json"})";
  }

  try {
    if (!body.HasMember("user_login") || !body.HasMember("pickup_address") ||
        !body.HasMember("destination_address")) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"user_login, pickup_address and destination_address are required"})";
    }

    const auto user_login = body["user_login"].As<std::string>();
    const auto pickup_address = body["pickup_address"].As<std::string>();
    const auto destination_address = body["destination_address"].As<std::string>();

    if (user_login.empty() || pickup_address.empty() || destination_address.empty()) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"fields must not be empty"})";
    }

    int amount = 500;
    if (body.HasMember("amount")) {
      amount = body["amount"].As<int>();
    }

    if (amount <= 0) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"amount must be positive"})";
    }

    using Oid = userver::formats::bson::Oid;
    using userver::formats::bson::MakeArray;
    using userver::formats::bson::MakeDoc;

    auto users = pool_->GetCollection("users");
    auto rides = pool_->GetCollection("rides");

    const auto user = users.FindOne(MakeDoc("login", user_login));
    if (!user) {
      response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return R"({"error":"mongo user not found"})";
    }

    const auto user_id = (*user)["_id"].As<Oid>();
    const auto first_name = (*user)["first_name"].As<std::string>();
    const auto last_name = (*user)["last_name"].As<std::string>();

    const auto now = std::chrono::system_clock::now();
    const Oid ride_id{};

    rides.InsertOne(MakeDoc(
        "_id", ride_id,
        "user_id", user_id,
        "status", "created",
        "created_at", now,
        "route", MakeDoc(
            "pickup_address", pickup_address,
            "dropoff_address", destination_address
        ),
        "fare", MakeDoc(
            "amount", amount,
            "currency", "RUB"
        ),
        "user_snapshot", MakeDoc(
            "login", user_login,
            "first_name", first_name,
            "last_name", last_name
        ),
        "events", MakeArray(
            MakeDoc("type", "created", "at", now)
        )
    ));

    response.SetStatus(userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(
        MakeRideResponse(
            ride_id.ToString(),
            user_login,
            pickup_address,
            destination_address,
            amount,
            "created"));
  } catch (const std::exception&) {
    response.SetStatus(userver::server::http::HttpStatus::kInternalServerError);
    return R"({"error":"failed to create mongo ride"})";
  }
}

userver::yaml_config::Schema MongoRidesCreate::GetStaticConfigSchema() {
  return userver::server::handlers::HttpHandlerBase::GetStaticConfigSchema();
}

}  // namespace taxi
