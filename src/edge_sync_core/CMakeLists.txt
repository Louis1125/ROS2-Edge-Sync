cmake_minimum_required(VERSION 3.20)
project(edge_sync_core VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(edge_sync_base REQUIRED)

add_library(edge_sync_core
  src/version.cpp
  src/sync_coordinator.cpp
  src/manifest_builder.cpp
  src/sync_planner.cpp
  src/sync_engine.cpp
)
add_library(edge_sync::core ALIAS edge_sync_core)
set_target_properties(edge_sync_core PROPERTIES EXPORT_NAME core)
target_compile_features(edge_sync_core PUBLIC cxx_std_20)
target_include_directories(edge_sync_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge_sync_core PUBLIC edge_sync::base)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_executable(test_edge_sync_core test/test_version.cpp test/test_manifest.cpp test/test_planner.cpp)
  target_link_libraries(test_edge_sync_core PRIVATE edge_sync_core GTest::gtest GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(test_edge_sync_core)
endif()

install(TARGETS edge_sync_core EXPORT edge_sync_coreTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT edge_sync_coreTargets
  FILE edge_sync_coreTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_core
)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_coreConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_coreConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_core
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_coreConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_coreConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_coreConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_core
)
