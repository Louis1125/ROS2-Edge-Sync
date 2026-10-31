#pragma once

#include <memory>

#include "edge_sync_base/result.hpp"
#include "edge_sync_connection/peer.hpp"
#include "edge_sync_connection/session.hpp"
#include "edge_sync_connection/transport_interface.hpp"

namespace edge_sync::connection {

class ConnectionManager {
public:
    explicit ConnectionManager(std::unique_ptr<ITransport> transport);

    edge_sync::base::Result<void> start();
    edge_sync::base::Result<Session::Id> open_session(const Peer& peer);
    edge_sync::base::Result<void> close_session(Session::Id id);
    edge_sync::base::Result<void> shutdown();

private:
    std::unique_ptr<ITransport> transport_;
};

}  // namespace edge_sync::connection
