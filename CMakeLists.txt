cmake_minimum_required(VERSION 3.12...3.31)

project(taxi_service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DownloadUserver)

find_package(
  userver
  COMPONENTS core postgresql mongo
  QUIET
)

if(NOT userver_FOUND)
  download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

include_directories(src)

add_library(
  ${PROJECT_NAME}_objs OBJECT
  src/storage/taxi_storage.cpp
  src/storage/taxi_storage_component.cpp
  src/handlers/users_create.cpp
  src/handlers/users_get.cpp
  src/handlers/auth_login.cpp
  src/handlers/drivers_create.cpp
  src/handlers/rides_create.cpp
  src/handlers/rides_get.cpp
  src/handlers/rides_accept.cpp
  src/handlers/rides_complete.cpp
  src/handlers/mongo_users_create.cpp
  src/handlers/mongo_users_get.cpp
  src/middlewares/auth_middleware.cpp
)

target_link_libraries(
  ${PROJECT_NAME}_objs
  PUBLIC
  userver::core
  userver::postgresql
  userver::mongo
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

userver_testsuite_add_simple()

include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml
  ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json
)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${PROJECT_NAME}
)

install(
  FILES ${CONFIGS_FILES}
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
  COMPONENT ${PROJECT_NAME}
)
