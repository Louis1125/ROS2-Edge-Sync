#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_connection/endpoint.hpp"

namespace edge_sync::transport {

class TcpServer {
public:
    edge_sync::base::Result<void> listen(const edge_sync::connection::Endpoint& endpoint);
};

}  // namespace edge_sync::transport
