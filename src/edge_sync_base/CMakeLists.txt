cmake_minimum_required(VERSION 3.20)
project(edge_sync_base VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(edge_sync_base
  src/error.cpp
  src/config.cpp
  src/logger.cpp
  src/sha256.cpp
  src/hmac.cpp
)
add_library(edge_sync::base ALIAS edge_sync_base)
set_target_properties(edge_sync_base PROPERTIES EXPORT_NAME base)

target_compile_features(edge_sync_base PUBLIC cxx_std_20)
target_include_directories(edge_sync_base PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_executable(test_edge_sync_base test/test_result.cpp test/test_config.cpp test/test_sha256.cpp test/test_hmac.cpp)
  target_link_libraries(test_edge_sync_base PRIVATE edge_sync_base GTest::gtest GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(test_edge_sync_base)
endif()

install(TARGETS edge_sync_base EXPORT edge_sync_baseTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT edge_sync_baseTargets
  FILE edge_sync_baseTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_base
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_baseConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_baseConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_base
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_baseConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_baseConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_baseConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_base
)
