#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "capturing_logger.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"

using edge_sync::connection::ConnectionManager;
using edge_sync::connection::Endpoint;
using edge_sync::connection::TcpTransport;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kServer{"127.0.0.1", 22443};

void wait_for(const std::function<bool()>& predicate, int attempts = 100) {
    for (int i = 0; i < attempts && !predicate(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}  // namespace

TEST(HandshakeAuthTest, CorrectSecretConnects) {
    CapturingLogger log;
    auto server = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    server->set_local_identity("robot-server");
    server->set_shared_secret("fleet-secret");
    ASSERT_TRUE(server->start(kServer).ok());

    auto client = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    client->set_local_identity("robot-client");
    client->set_shared_secret("fleet-secret");
    ASSERT_TRUE(client->start(Endpoint{"127.0.0.1", 22444}).ok());
    ASSERT_TRUE(client->connect_peer(kServer).ok());

    wait_for([&] {
        return server->is_connected("robot-client") && client->is_connected("robot-server");
    });
    EXPECT_TRUE(server->is_connected("robot-client"));
    EXPECT_TRUE(client->is_connected("robot-server"));

    client->stop();
    server->stop();
}

TEST(HandshakeAuthTest, WrongSecretRejected) {
    CapturingLogger log;
    auto server = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    server->set_local_identity("robot-server");
    server->set_shared_secret("fleet-secret");
    ASSERT_TRUE(server->start(kServer).ok());

    auto client = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    client->set_local_identity("robot-client");
    client->set_shared_secret("wrong-secret");
    ASSERT_TRUE(client->start(Endpoint{"127.0.0.1", 22445}).ok());
    ASSERT_TRUE(client->connect_peer(kServer).ok());

    wait_for([&] { return log.contains("authentication failed"); });
    EXPECT_FALSE(server->is_connected("robot-client"));

    client->stop();
    server->stop();
}
