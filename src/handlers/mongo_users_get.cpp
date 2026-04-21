#include "mongo_users_get.hpp"

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

userver::formats::json::Value MakeUserJson(const userver::formats::bson::Value& doc) {
  userver::formats::json::ValueBuilder builder;

  if (doc.HasMember("login")) builder["login"] = doc["login"].As<std::string>();
  if (doc.HasMember("first_name")) builder["first_name"] = doc["first_name"].As<std::string>();
  if (doc.HasMember("last_name")) builder["last_name"] = doc["last_name"].As<std::string>();
  if (doc.HasMember("status")) builder["status"] = doc["status"].As<std::string>();

  if (doc.HasMember("profile")) {
    const auto profile = doc["profile"];
    if (profile.HasMember("email")) builder["profile"]["email"] = profile["email"].As<std::string>();
    if (profile.HasMember("phone")) builder["profile"]["phone"] = profile["phone"].As<std::string>();
  }

  builder["source"] = "mongo";
  return builder.ExtractValue();
}

}  // namespace

MongoUsersGet::MongoUsersGet(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

std::string MongoUsersGet::HandleRequestThrow(
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
    return R"({"error":"use exactly one query parameter: login or name_mask"})";
  }

  try {
    using userver::formats::bson::MakeArray;
    using userver::formats::bson::MakeDoc;

    auto users = pool_->GetCollection("users");

    if (has_login) {
      const auto user = users.FindOne(MakeDoc("login", login));
      if (!user) {
        response.SetStatus(userver::server::http::HttpStatus::kNotFound);
        return R"({"error":"mongo user not found"})";
      }

      response.SetStatus(userver::server::http::HttpStatus::kOk);
      return userver::formats::json::ToString(MakeUserJson(*user));
    }

    const auto query = MakeDoc(
        "$or", MakeArray(
            MakeDoc("first_name", MakeDoc("$regex", name_mask, "$options", "i")),
            MakeDoc("last_name", MakeDoc("$regex", name_mask, "$options", "i"))
        )
    );

    auto cursor = users.Find(query);

    userver::formats::json::ValueBuilder result(userver::formats::common::Type::kArray);
    for (const auto& doc : cursor) {
      result.PushBack(MakeUserJson(doc));
    }

    response.SetStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(result.ExtractValue());
  } catch (const std::exception&) {
    response.SetStatus(userver::server::http::HttpStatus::kInternalServerError);
    return R"({"error":"failed to read mongo users"})";
  }
}

userver::yaml_config::Schema MongoUsersGet::GetStaticConfigSchema() {
  return userver::server::handlers::HttpHandlerBase::GetStaticConfigSchema();
}

}  // namespace taxi
