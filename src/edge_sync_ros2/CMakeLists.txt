cmake_minimum_required(VERSION 3.20)
project(edge_sync_ros2 LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(edge_sync_interfaces REQUIRED)

add_library(${PROJECT_NAME} src/edge_sync_node.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(${PROJECT_NAME} rclcpp edge_sync_interfaces)

add_executable(edge_sync_node src/node_main.cpp)
target_compile_features(edge_sync_node PRIVATE cxx_std_20)
target_include_directories(edge_sync_node PRIVATE include)
target_link_libraries(edge_sync_node ${PROJECT_NAME})
ament_target_dependencies(edge_sync_node rclcpp edge_sync_interfaces)

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(TARGETS edge_sync_node RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp edge_sync_interfaces)

ament_package()
