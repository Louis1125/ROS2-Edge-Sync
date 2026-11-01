cmake_minimum_required(VERSION 3.20)
project(edge_sync_connection VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(edge_sync_base REQUIRED)
find_package(edge_sync_protocol REQUIRED)
find_package(edge_sync_core REQUIRED)

# --- Pure target: interfaces + ConnectionManager, no concrete transport. ---
add_library(edge_sync_connection
  src/frame_codec.cpp
  src/connection_manager.cpp
  src/sync_peer_adapter.cpp
)
add_library(edge_sync::connection ALIAS edge_sync_connection)
set_target_properties(edge_sync_connection PROPERTIES EXPORT_NAME connection)
target_compile_features(edge_sync_connection PUBLIC cxx_std_20)
target_include_directories(edge_sync_connection PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge_sync_connection PUBLIC edge_sync::base edge_sync::protocol edge_sync::core)

# --- Asio transport target: implements ITransport over TCP. ---
find_package(Boost REQUIRED)

add_library(edge_sync_connection_asio src/tcp_transport.cpp)
add_library(edge_sync::connection_asio ALIAS edge_sync_connection_asio)
set_target_properties(edge_sync_connection_asio PROPERTIES EXPORT_NAME connection_asio)
target_compile_features(edge_sync_connection_asio PUBLIC cxx_std_20)
target_include_directories(edge_sync_connection_asio PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(edge_sync_connection_asio PUBLIC
  edge_sync::connection
  edge_sync::base
  edge_sync::protocol
  Boost::headers
)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  find_package(edge_sync_storage REQUIRED)
  set(TEST_SUPPORT ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/support)

  add_executable(test_frame_codec test/test_frame_codec.cpp)
  target_link_libraries(test_frame_codec PRIVATE edge_sync_connection GTest::gtest GTest::gtest_main)

  add_executable(test_handshake test/test_handshake.cpp)
  target_link_libraries(test_handshake PRIVATE edge_sync_connection edge_sync_connection_asio GTest::gtest GTest::gtest_main)
  target_include_directories(test_handshake PRIVATE ${TEST_SUPPORT})

  add_executable(test_handshake_auth test/test_handshake_auth.cpp)
  target_link_libraries(test_handshake_auth PRIVATE edge_sync_connection edge_sync_connection_asio GTest::gtest GTest::gtest_main)
  target_include_directories(test_handshake_auth PRIVATE ${TEST_SUPPORT})

  add_executable(test_agent_command test/test_agent_command.cpp)
  target_link_libraries(test_agent_command PRIVATE edge_sync_connection edge_sync_connection_asio GTest::gtest GTest::gtest_main)
  target_include_directories(test_agent_command PRIVATE ${TEST_SUPPORT})

  add_executable(test_reconnect test/test_reconnect.cpp)
  target_link_libraries(test_reconnect PRIVATE edge_sync_connection edge_sync_connection_asio GTest::gtest GTest::gtest_main)
  target_include_directories(test_reconnect PRIVATE ${TEST_SUPPORT})

  add_executable(test_sync test/test_sync.cpp)
  target_link_libraries(test_sync PRIVATE
    edge_sync_connection edge_sync_connection_asio edge_sync::storage edge_sync::core
    GTest::gtest GTest::gtest_main)
  target_include_directories(test_sync PRIVATE ${TEST_SUPPORT})

  add_executable(test_transfer test/test_transfer.cpp)
  target_link_libraries(test_transfer PRIVATE
    edge_sync_connection edge_sync_connection_asio edge_sync::storage edge_sync::core
    GTest::gtest GTest::gtest_main)
  target_include_directories(test_transfer PRIVATE ${TEST_SUPPORT})

  add_executable(test_sync_engine test/test_sync_engine.cpp)
  target_link_libraries(test_sync_engine PRIVATE
    edge_sync_connection edge_sync_connection_asio edge_sync::storage edge_sync::core
    GTest::gtest GTest::gtest_main)
  target_include_directories(test_sync_engine PRIVATE ${TEST_SUPPORT})

  include(GoogleTest)
  gtest_discover_tests(test_frame_codec)
  gtest_discover_tests(test_handshake)
  gtest_discover_tests(test_handshake_auth)
  gtest_discover_tests(test_agent_command)
  gtest_discover_tests(test_reconnect)
  gtest_discover_tests(test_sync)
  gtest_discover_tests(test_transfer)
  gtest_discover_tests(test_sync_engine)
endif()

install(TARGETS edge_sync_connection EXPORT edge_sync_connectionTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS edge_sync_connection_asio EXPORT edge_sync_connection_asioTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT edge_sync_connectionTargets
  FILE edge_sync_connectionTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection
)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_connectionConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connectionConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connectionConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connectionConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connectionConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection
)

install(EXPORT edge_sync_connection_asioTargets
  FILE edge_sync_connection_asioTargets.cmake
  NAMESPACE edge_sync::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection_asio
)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edge_sync_connection_asioConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connection_asioConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection_asio
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connection_asioConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connection_asioConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/edge_sync_connection_asioConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edge_sync_connection_asio
)
