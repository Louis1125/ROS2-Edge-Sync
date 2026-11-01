#include "edge_sync_control_plane/application/agent_gateway.hpp"

#include <chrono>

namespace edge_sync::control_plane {

AgentGateway::AgentGateway(Database& db, edge_sync::base::Logger& logger)
    : db_(db), logger_(logger) {}

edge_sync::base::Result<void> AgentGateway::start(std::uint16_t port,
                                                  const std::string& fleet_secret) {
    connections_ = std::make_unique<edge_sync::connection::ConnectionManager>(
        std::make_unique<edge_sync::connection::TcpTransport>(), logger_);
    connections_->set_local_identity("control-plane");
    if (!fleet_secret.empty()) connections_->set_shared_secret(fleet_secret);

    connections_->set_peer_event_handler(
        [this](const std::string& robot_id) { on_peer_connected(robot_id); },
        [this](const std::string& robot_id) { on_peer_disconnected(robot_id); });
    connections_->set_application_message_handler(
        [this](const edge_sync::connection::Peer& peer,
               const edge_sync::protocol::MessageEnvelope& env) { on_message(peer, env); });

    if (auto r = connections_->start({"0.0.0.0", port}); !r) return r.error();

    dispatcher_ = std::jthread([this](std::stop_token stop) { dispatcher_loop(stop); });
    logger_.log(edge_sync::base::LogLevel::Info,
                "agent gateway listening on port " + std::to_string(port));
    return {};
}

void AgentGateway::stop() {
    if (stopping_.exchange(true)) return;
    if (dispatcher_.joinable()) {
        dispatcher_.request_stop();
        dispatcher_.join();
    }
    if (connections_) connections_->stop();
}

void AgentGateway::on_peer_connected(const std::string& robot_id) {
    db_.upsert_agent_session(robot_id);
    db_.touch_robot(robot_id);
    logger_.log(edge_sync::base::LogLevel::Info, "agent connected: " + robot_id);
}

void AgentGateway::on_peer_disconnected(const std::string& robot_id) {
    logger_.log(edge_sync::base::LogLevel::Warn, "agent disconnected: " + robot_id);
}

void AgentGateway::on_message(const edge_sync::connection::Peer& peer,
                              const edge_sync::protocol::MessageEnvelope& env) {
    if (env.has_heartbeat()) {
        db_.touch_robot(peer.identity.robot_id);
        return;
    }
    if (!env.has_sync_status_report()) return;
    const auto& report = env.sync_status_report();
    const auto status = report.status();
    logger_.log(edge_sync::base::LogLevel::Info,
                "agent status for " + report.command_id() + ": " + status);

    if (status == "acknowledged") {
        db_.mark_outbox(report.command_id(), "acknowledged");
    } else if (status == "succeeded" || status == "failed" || status == "cancelled") {
        db_.update_job_execution(report.command_id(), status, report.error());
        db_.mark_outbox(report.command_id(), "acknowledged");
    }
}

void AgentGateway::dispatcher_loop(std::stop_token stop) {
    while (!stop.stop_requested() && !stopping_) {
        auto entries = db_.poll_outbox();
        if (entries.ok()) {
            for (const auto& e : entries.value()) {
                if (!connections_ || !connections_->is_connected(e.target_robot)) continue;
                edge_sync::protocol::MessageEnvelope cmd;
                cmd.set_protocol_version(edge_sync::protocol::kProtocolVersion);
                cmd.mutable_sync_command()->set_command_id(e.command_id);
                cmd.mutable_sync_command()->set_source_robot(e.source_robot);
                cmd.mutable_sync_command()->set_artifact_id(e.artifact_id);
                connections_->send(e.target_robot, cmd);
                db_.mark_outbox(e.command_id, "dispatched");
                logger_.log(edge_sync::base::LogLevel::Info,
                            "dispatched " + e.command_id + " to " + e.target_robot);
            }
        }
        for (int i = 0; i < 10 && !stop.stop_requested(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

}  // namespace edge_sync::control_plane
