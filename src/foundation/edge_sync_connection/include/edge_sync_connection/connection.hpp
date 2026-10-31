#pragma once

#include <utility>

#include "edge_sync_connection/peer.hpp"

namespace edge_sync::connection {

class Connection {
public:
    explicit Connection(Peer peer) : peer_(std::move(peer)) {}

    [[nodiscard]] const Peer& peer() const noexcept { return peer_; }
    [[nodiscard]] ConnectionState state() const noexcept { return peer_.state; }

private:
    Peer peer_;
};

}  // namespace edge_sync::connection
