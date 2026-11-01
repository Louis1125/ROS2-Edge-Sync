cmake_minimum_required(VERSION 3.20)
project(edge_sync_runtime VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

find_package(edge_sync_base REQUIRED)
find_package(edge_sync_protocol REQUIRED)
find_package(edge_sync_core REQUIRED)
find_package(edge_sync_connection REQUIRED)
find_package(edge_sync_connection_asio REQUIRED)
find_package(edge_sync_storage REQUIRED)
set(CMAKE_CXX_STANDARD 20)

add_executable(edge_syncd src/main.cpp)
target_compile_features(edge_syncd PRIVATE cxx_std_20)
target_include_directories(edge_syncd PRIVATE include)
target_link_libraries(edge_syncd PRIVATE
  edge_sync::base
  edge_sync::protocol
  edge_sync::core
  edge_sync::connection
  edge_sync::connection_asio
  edge_sync::storage
)

install(TARGETS edge_syncd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
