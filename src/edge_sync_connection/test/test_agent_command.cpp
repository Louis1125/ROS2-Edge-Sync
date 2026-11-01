#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"

#include "capturing_logger.hpp"

using namespace edge_sync::connection;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kControlPlane{"127.0.0.1", 23443};

void wait_for(const std::function<bool()>& predicate, int attempts = 100) {
    for (int i = 0; i < attempts && !predicate(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}  // namespace

// Control plane dispatches a SyncCommand to a connected robot; the robot
// receives it, executes (here: acknowledges + reports), and the control plane
// records the execution status. This is the core agent command lifecycle.
TEST(AgentCommandTest, DispatchAndStatusRoundTrip) {
    CapturingLogger log;

    std::mutex mu;
    std::string reported_status;

    auto cp = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    cp->set_local_identity("control-plane");
    cp->set_shared_secret("fleet-secret");
    cp->set_application_message_handler([&](const Peer&, const edge_sync::protocol::MessageEnvelope& env) {
        if (env.has_sync_status_report()) {
            std::lock_guard lk(mu);
            reported_status = env.sync_status_report().status();
        }
    });
    ASSERT_TRUE(cp->start(kControlPlane).ok());

    std::string received_command_id;
    std::string received_artifact;
    auto robot = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    robot->set_local_identity("robot-a");
    robot->set_shared_secret("fleet-secret");
    robot->set_application_message_handler([&](const Peer& peer, const edge_sync::protocol::MessageEnvelope& env) {
        if (env.has_sync_command()) {
            received_command_id = env.sync_command().command_id();
            received_artifact = env.sync_command().artifact_id();
            edge_sync::protocol::MessageEnvelope report;
            report.set_protocol_version(edge_sync::protocol::kProtocolVersion);
            report.mutable_sync_status_report()->set_command_id(env.sync_command().command_id());
            report.mutable_sync_status_report()->set_status("succeeded");
            robot->send(peer.identity.robot_id, report);
        }
    });
    ASSERT_TRUE(robot->start(Endpoint{"127.0.0.1", 23444}).ok());
    ASSERT_TRUE(robot->connect_peer(kControlPlane).ok());
    wait_for([&] { return cp->is_connected("robot-a") && robot->is_connected("control-plane"); });
    ASSERT_TRUE(cp->is_connected("robot-a"));

    edge_sync::protocol::MessageEnvelope cmd;
    cmd.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    cmd.mutable_sync_command()->set_command_id("cmd-1");
    cmd.mutable_sync_command()->set_source_robot("robot-b");
    cmd.mutable_sync_command()->set_artifact_id("map-1");
    cp->send("robot-a", cmd);

    wait_for([&] { std::lock_guard lk(mu); return !reported_status.empty(); });
    EXPECT_EQ(received_command_id, "cmd-1");
    EXPECT_EQ(received_artifact, "map-1");
    {
        std::lock_guard lk(mu);
        EXPECT_EQ(reported_status, "succeeded");
    }

    robot->stop();
    cp->stop();
}
