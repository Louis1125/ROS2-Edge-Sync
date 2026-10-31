cmake_minimum_required(VERSION 3.20)
project(edge_sync_protocol LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(edge_sync_base REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_FILES
  ${PROTO_DIR}/v1/common.proto
  ${PROTO_DIR}/v1/sync.proto
  ${PROTO_DIR}/v1/manifest.proto
  ${PROTO_DIR}/v1/transfer.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(${PROJECT_NAME} SHARED
  ${PROTO_SRCS}
  ${PROTO_HDRS}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${PROTO_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME} PUBLIC edge_sync_base protobuf::libprotobuf)

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ DESTINATION include
  FILES_MATCHING PATTERN "*.pb.h")

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(edge_sync_base Protobuf)

ament_package()
