#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "edge_sync_base/logger.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"
#include "edge_sync_control_plane/infrastructure/database.hpp"

namespace edge_sync::control_plane {

// Agent gateway: accepts outbound authenticated robot connections, keeps an
// agent session registry, and durably dispatches sync commands from the
// transactional outbox. Delivery and execution states are tracked separately.
class AgentGateway {
public:
    AgentGateway(Database& db, edge_sync::base::Logger& logger);

    edge_sync::base::Result<void> start(std::uint16_t port, const std::string& fleet_secret);
    void stop();

private:
    void dispatcher_loop(std::stop_token stop);
    void on_peer_connected(const std::string& robot_id);
    void on_peer_disconnected(const std::string& robot_id);
    void on_message(const edge_sync::connection::Peer& peer,
                    const edge_sync::protocol::MessageEnvelope& env);

    Database& db_;
    edge_sync::base::Logger& logger_;
    std::unique_ptr<edge_sync::connection::ConnectionManager> connections_;
    std::atomic<bool> stopping_{false};
    std::jthread dispatcher_;
};

}  // namespace edge_sync::control_plane
