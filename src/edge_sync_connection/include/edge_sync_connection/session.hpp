#pragma once

#include <cstdint>
#include <string>

#include "edge_sync_connection/peer.hpp"

namespace edge_sync::connection {

// A logical synchronization session bound to a connected peer. The transport
// connection that carries it is owned by ConnectionManager; a session is a
// lightweight handle so upper layers never touch sockets directly.
class Session {
public:
    using Id = std::uint64_t;

    Session(Id id, Peer peer) : id_(id), peer_(std::move(peer)) {}

    [[nodiscard]] Id id() const noexcept { return id_; }
    [[nodiscard]] const Peer& peer() const noexcept { return peer_; }

private:
    Id id_;
    Peer peer_;
};

}  // namespace edge_sync::connection
