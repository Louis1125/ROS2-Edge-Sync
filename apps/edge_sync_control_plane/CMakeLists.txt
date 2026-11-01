cmake_minimum_required(VERSION 3.20)
project(edge_sync_control_plane VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(edge_sync_base REQUIRED)
find_package(edge_sync_core REQUIRED)
find_package(Drogon REQUIRED)
find_package(PostgreSQL REQUIRED)

add_executable(edge_sync_control_plane
  src/main.cpp
  src/database.cpp
  src/robot_service.cpp
  src/sync_job_service.cpp
)
target_include_directories(edge_sync_control_plane PRIVATE include)
target_link_libraries(edge_sync_control_plane PRIVATE
  edge_sync::base
  edge_sync::core
  Drogon::Drogon
  PostgreSQL::PostgreSQL
)

install(TARGETS edge_sync_control_plane RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
