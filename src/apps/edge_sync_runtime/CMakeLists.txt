cmake_minimum_required(VERSION 3.20)
project(edge_sync_runtime LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(edge_sync_base REQUIRED)
find_package(edge_sync_connection REQUIRED)
find_package(edge_sync_application REQUIRED)
find_package(edge_sync_ros2 REQUIRED)

add_executable(edge_syncd
  src/main.cpp
  src/application_builder.cpp
  src/shutdown_handler.cpp
)

target_compile_features(edge_syncd PRIVATE cxx_std_20)
target_include_directories(edge_syncd PRIVATE include)
ament_target_dependencies(edge_syncd
  rclcpp
  edge_sync_base
  edge_sync_connection
  edge_sync_application
  edge_sync_ros2
)

install(TARGETS edge_syncd
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_package()
