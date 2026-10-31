cmake_minimum_required(VERSION 3.20)
project(edge_sync_protocol VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(edge_sync_base REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_FILES
  v1/common.proto
  v1/sync.proto
  v1/manifest.proto
  v1/transfer.proto
)

# Invoke protoc explicitly so the --proto_path is fully controlled.
set(PROTO_GENERATED_SRCS)
foreach(PROTO_FILE ${PROTO_FILES})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" GEN_CC ${PROTO_FILE})
  string(REGEX REPLACE "\\.proto$" ".pb.h" GEN_HDR ${PROTO_FILE})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GEN_CC} ${CMAKE_CURRENT_BINARY_DIR}/${GEN_HDR}
    COMMAND protobuf::protoc
      --proto_path=${PROTO_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      ${PROTO_FILE}
    DEPENDS ${PROTO_DIR}/${PROTO_FILE} protobuf::protoc
    COMMENT "Generating C++ from ${PROTO_FILE}"
    VERBATIM
  )
  list(APPEND PROTO_GENERATED_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${GEN_CC})
endforeach()

add_library(edge_sync_protocol
  ${PROTO_GENERATED_SRCS}
  src/message_codec.cpp
)
add_library(edge_sync::protocol ALIAS edge_sync_protocol)
set_target_properties(edge_sync_protocol PROPERTIES EXPORT_NAME protocol)

target_compile_features(edge_sync_protocol PUBLIC cxx_std_20)
target_include_directories(edge_sync_protocol PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge_sync_protocol PUBLIC edge_sync::base protobuf::libprotobuf)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_executable(test_edge_sync_protocol test/test_protocol.cpp)
  target_link_libraries(test_edge_sync_protocol PRIVATE edge_sync_protocol GTest::gtest GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(test_edge_sync_protocol)
endif()

install(TARGETS edge_sync_protocol EXPORT edge_sync_protocolTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.pb.h")

install(EXPORT edge_sync_protocolTargets
  FILE edge_sync_protocolTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_protocol
)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_protocolConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_protocolConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_protocol
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_protocolConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_protocolConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_protocolConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_protocol
)
