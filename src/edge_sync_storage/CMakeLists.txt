cmake_minimum_required(VERSION 3.20)
project(edge_sync_storage VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(edge_sync_base REQUIRED)
find_package(edge_sync_core REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(edge_sync_storage
  src/object_store.cpp
  src/sqlite_repository.cpp
  src/artifact_repository.cpp
  src/artifact_store.cpp
)
add_library(edge_sync::storage ALIAS edge_sync_storage)
set_target_properties(edge_sync_storage PROPERTIES EXPORT_NAME storage)
target_compile_features(edge_sync_storage PUBLIC cxx_std_20)
target_include_directories(edge_sync_storage PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge_sync_storage PUBLIC edge_sync::base edge_sync::core SQLite::SQLite3)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_executable(test_edge_sync_storage test/test_storage.cpp test/test_artifact_store.cpp)
  target_link_libraries(test_edge_sync_storage PRIVATE edge_sync_storage GTest::gtest GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(test_edge_sync_storage)
endif()

install(TARGETS edge_sync_storage EXPORT edge_sync_storageTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT edge_sync_storageTargets
  FILE edge_sync_storageTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_storage
)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_storageConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_storageConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_storage
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_storageConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_storageConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_storageConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_storage
)
