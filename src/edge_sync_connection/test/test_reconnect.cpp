#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "capturing_logger.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"

using edge_sync::connection::ConnectionManager;
using edge_sync::connection::Endpoint;
using edge_sync::connection::RetryPolicy;
using edge_sync::connection::TcpTransport;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kServer{"127.0.0.1", 18443};

void wait_for(std::function<bool()> predicate, int attempts = 100) {
    for (int i = 0; i < attempts && !predicate(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}  // namespace

TEST(ReconnectTest, ClientDetectsDropAndRetries) {
    CapturingLogger server_log;
    CapturingLogger client_log;

    RetryPolicy retry;
    retry.max_attempts = 4;
    retry.base_delay = std::chrono::milliseconds(20);
    retry.max_delay = std::chrono::milliseconds(200);

    auto server = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), server_log);
    server->set_local_identity("robot-server");
    ASSERT_TRUE(server->start(kServer).ok());

    auto client =
        std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), client_log, retry);
    client->set_local_identity("robot-client");
    ASSERT_TRUE(client->start(Endpoint{"127.0.0.1", 18444}).ok());
    ASSERT_TRUE(client->connect_peer(kServer).ok());

    wait_for([&] { return client->is_connected("robot-server"); });
    ASSERT_TRUE(client->is_connected("robot-server"));

    // Kill the server; the client must detect the drop and attempt reconnects.
    server->stop();
    wait_for([&] { return !client->is_connected("robot-server"); });

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_TRUE(client_log.contains("reconnect attempt"));
    EXPECT_FALSE(client->is_connected("robot-server"));

    client->stop();
}
