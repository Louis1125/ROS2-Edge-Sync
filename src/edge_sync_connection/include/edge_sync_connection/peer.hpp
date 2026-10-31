#pragma once

#include <string>

#include "edge_sync_connection/endpoint.hpp"

namespace edge_sync::connection {

struct RobotIdentity {
    std::string robot_id;
    std::string public_key_id;
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Failed,
};

struct Peer {
    RobotIdentity identity;
    Endpoint endpoint;
    ConnectionState state{ConnectionState::Disconnected};
};

}  // namespace edge_sync::connection
