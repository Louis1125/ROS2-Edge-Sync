cmake_minimum_required(VERSION 3.20)
project(edge_sync_interfaces LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SyncStatus.msg"
  "srv/TriggerSync.srv"
  "action/SyncArtifact.action"
  DEPENDENCIES builtin_interfaces std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
