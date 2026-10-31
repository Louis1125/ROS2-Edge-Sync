cmake_minimum_required(VERSION 3.20)
project(edge_sync_cli LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(edge_sync_base REQUIRED)
find_package(edge_sync_application REQUIRED)

add_executable(edge-sync src/main.cpp)

target_compile_features(edge-sync PRIVATE cxx_std_20)
target_include_directories(edge-sync PRIVATE include)
ament_target_dependencies(edge-sync edge_sync_base edge_sync_application)

install(TARGETS edge-sync
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_package()
