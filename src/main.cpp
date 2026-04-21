#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/auth_login.hpp"
#include "handlers/drivers_create.hpp"
#include "handlers/mongo_users_create.hpp"
#include "handlers/mongo_users_get.hpp"
#include "handlers/rides_accept.hpp"
#include "handlers/rides_complete.hpp"
#include "handlers/rides_create.hpp"
#include "handlers/rides_get.hpp"
#include "handlers/users_create.hpp"
#include "handlers/users_get.hpp"
#include "middlewares/auth_middleware.hpp"
#include "storage/taxi_storage_component.hpp"

int main(int argc, char* argv[]) {
  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .AppendComponentList(userver::clients::http::ComponentList())
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::congestion_control::Component>()
          .Append<userver::components::Postgres>("postgres-db")
          .Append<userver::components::Mongo>("mongo-db")
          .Append<taxi::TaxiStorageComponent>()
          .Append<taxi::UsersCreate>()
          .Append<taxi::UsersGet>()
          .Append<taxi::AuthLogin>()
          .Append<taxi::DriversCreate>()
          .Append<taxi::RidesCreate>()
          .Append<taxi::RidesGet>()
          .Append<taxi::RidesAccept>()
          .Append<taxi::RidesComplete>()
          .Append<taxi::MongoUsersCreate>()
          .Append<taxi::MongoUsersGet>()
          .Append<taxi::AuthMiddlewareFactory>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}
