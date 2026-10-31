cmake_minimum_required(VERSION 3.20)
project(edge_sync_storage LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(edge_sync_base REQUIRED)
find_package(edge_sync_domain REQUIRED)
find_package(SQLite3 REQUIRED)

# Zstandard is a dependency for chunk compression (M4). It is not yet linked
# while storage is a skeleton; wire it up as zstd::libzstd_shared when needed.

add_library(${PROJECT_NAME} SHARED src/object_store.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME} PUBLIC edge_sync_base edge_sync_domain SQLite::SQLite3)

install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(edge_sync_base edge_sync_domain SQLite3)

ament_package()
