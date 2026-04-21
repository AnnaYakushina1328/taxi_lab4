#include "mongo_users_create.hpp"

#include <chrono>
#include <regex>
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

bool IsValidLogin(const std::string& login) {
  static const std::regex pattern("^[a-z0-9._-]+$");
  return std::regex_match(login, pattern);
}

}  // namespace

MongoUsersCreate::MongoUsersCreate(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

std::string MongoUsersCreate::HandleRequestThrow(
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
    if (!body.HasMember("login") || !body.HasMember("password") ||
        !body.HasMember("first_name") || !body.HasMember("last_name")) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"login, password, first_name and last_name are required"})";
    }

    const auto login = body["login"].As<std::string>();
    const auto password = body["password"].As<std::string>();
    const auto first_name = body["first_name"].As<std::string>();
    const auto last_name = body["last_name"].As<std::string>();

    if (login.empty() || password.empty() || first_name.empty() || last_name.empty()) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"fields must not be empty"})";
    }

    if (!IsValidLogin(login)) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"login must contain only a-z, 0-9, ., _, -"})";
    }

    if (password.size() < 6) {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"password must be at least 6 characters"})";
    }

    std::string status = "active";
    if (body.HasMember("status")) {
      status = body["status"].As<std::string>();
    }

    if (status != "active" && status != "blocked") {
      response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return R"({"error":"status must be active or blocked"})";
    }

    std::string email = login + "@example.com";
    if (body.HasMember("email")) {
      email = body["email"].As<std::string>();
    }

    std::string phone = "+79990000000";
    if (body.HasMember("phone")) {
      phone = body["phone"].As<std::string>();
    }

    using userver::formats::bson::MakeDoc;

    auto users = pool_->GetCollection("users");

    if (users.FindOne(MakeDoc("login", login))) {
      response.SetStatus(userver::server::http::HttpStatus::kConflict);
      return R"({"error":"mongo user already exists"})";
    }

    users.InsertOne(MakeDoc(
        "login", login,
        "password", password,
        "first_name", first_name,
        "last_name", last_name,
        "created_at", std::chrono::system_clock::now(),
        "status", status,
        "profile", MakeDoc(
            "email", email,
            "phone", phone
        )
    ));

    userver::formats::json::ValueBuilder result;
    result["login"] = login;
    result["first_name"] = first_name;
    result["last_name"] = last_name;
    result["status"] = status;
    result["profile"]["email"] = email;
    result["profile"]["phone"] = phone;
    result["source"] = "mongo";

    response.SetStatus(userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(result.ExtractValue());
  } catch (const std::exception&) {
    response.SetStatus(userver::server::http::HttpStatus::kInternalServerError);
    return R"({"error":"failed to create mongo user"})";
  }
}

userver::yaml_config::Schema MongoUsersCreate::GetStaticConfigSchema() {
  return userver::server::handlers::HttpHandlerBase::GetStaticConfigSchema();
}

}  // namespace taxi
