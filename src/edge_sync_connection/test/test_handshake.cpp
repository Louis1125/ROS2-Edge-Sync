#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"

#include "capturing_logger.hpp"

using edge_sync::base::LogLevel;
using edge_sync::connection::ConnectionManager;
using edge_sync::connection::Endpoint;
using edge_sync::connection::TcpTransport;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kServer{"127.0.0.1", 17443};
const Endpoint kClient{"127.0.0.1", 17444};

void wait_for(std::function<bool()> predicate, int attempts = 100) {
    for (int i = 0; i < attempts && !predicate(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}  // namespace

TEST(HandshakeTest, TwoManagersCompleteHandshakeAndSession) {
    CapturingLogger server_log;
    CapturingLogger client_log;

    auto server = std::make_unique<ConnectionManager>(
        std::make_unique<TcpTransport>(), server_log);
    server->set_local_identity("robot-server");
    ASSERT_TRUE(server->start(kServer).ok());

    auto client = std::make_unique<ConnectionManager>(
        std::make_unique<TcpTransport>(), client_log);
    client->set_local_identity("robot-client");
    ASSERT_TRUE(client->start(kClient).ok());

    ASSERT_TRUE(client->connect_peer(kServer).ok());

    wait_for([&] { return server->is_connected("robot-client") && client->is_connected("robot-server"); });
    ASSERT_TRUE(server->is_connected("robot-client"));
    ASSERT_TRUE(client->is_connected("robot-server"));

    auto session = client->open_session("robot-server");
    ASSERT_TRUE(session.ok());
    ASSERT_TRUE(client->close_session(session.value()).ok());

    client->stop();
    server->stop();
}

TEST(HandshakeTest, ProtocolMismatchIsRejected) {
    CapturingLogger log;
    auto server = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    server->set_local_identity("robot-server");
    ASSERT_TRUE(server->start(kServer).ok());

    // Raw client that sends a handshake with a bogus protocol version.
    auto transport = std::make_unique<TcpTransport>();
    auto conn = transport->connect(kServer);
    ASSERT_TRUE(conn.ok());
    edge_sync::protocol::MessageEnvelope env;
    env.set_robot_id("bad-client");
    env.set_protocol_version(999);
    env.mutable_handshake_request()->set_robot_id("bad-client");
    env.mutable_handshake_request()->set_protocol_version(999);
    auto bytes = edge_sync::protocol::serialize(env);
    ASSERT_TRUE(bytes.ok());
    transport->send(conn.value(), bytes.value());

    wait_for([&] { return log.contains("version mismatch"); });
    EXPECT_TRUE(log.contains("version mismatch"));

    transport->stop();
    server->stop();
}
