#include "edge_sync_transport_asio/tcp_transport.hpp"

namespace edge_sync::transport {

edge_sync::base::Result<void> TcpTransport::start() { return {}; }

edge_sync::base::Result<void> TcpTransport::connect(
    const edge_sync::connection::Endpoint&) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                  "not implemented"};
}

void TcpTransport::async_send(const edge_sync::connection::ByteBuffer&,
                              edge_sync::connection::CompletionHandler handler) {
    handler(edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                   "not implemented"});
}

void TcpTransport::close() noexcept {}

}  // namespace edge_sync::transport
