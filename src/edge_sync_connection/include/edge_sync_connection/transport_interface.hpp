#pragma once

#include <cstdint>
#include <functional>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_connection/endpoint.hpp"

namespace edge_sync::connection {

using ConnectionId = std::uint64_t;
using ByteBuffer = edge_sync::base::ByteBuffer;

// Callbacks a transport invokes on its owner. All callbacks are invoked from
// the transport's I/O thread and must not block.
struct TransportCallbacks {
    // A remote endpoint accepted on the listening socket (inbound).
    std::function<void(ConnectionId, const Endpoint&)> on_accepted;
    // A complete, de-framed message from a connection.
    std::function<void(ConnectionId, ByteBuffer)> on_message;
    // A connection was closed by either side.
    std::function<void(ConnectionId)> on_disconnected;
};

// Transport-agnostic interface implemented by concrete transports (Asio TCP,
// and later HTTP/P2P). Upper layers never open sockets directly.
class ITransport {
   public:
    virtual ~ITransport() = default;

    // Begin listening on `listen_endpoint`; accepted connections fire
    // on_accepted. Returns once the acceptor is bound.
    virtual edge_sync::base::Result<void> start(const Endpoint& listen_endpoint) = 0;

    // Establish an outbound connection. Blocks until connected or timeout.
    virtual edge_sync::base::Result<ConnectionId> connect(const Endpoint& endpoint) = 0;

    // Asynchronously send a raw (already framed) payload.
    virtual void send(ConnectionId id, const ByteBuffer& data) = 0;

    virtual void close(ConnectionId id) = 0;
    virtual void stop() = 0;

    void set_callbacks(TransportCallbacks callbacks) {
        callbacks_ = std::move(callbacks);
    }

   protected:
    TransportCallbacks callbacks_;
};

}  // namespace edge_sync::connection
