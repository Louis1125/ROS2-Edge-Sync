#pragma once

#include "edge_sync_connection/transport_interface.hpp"

namespace edge_sync::transport {

class TcpTransport final : public edge_sync::connection::ITransport {
public:
    edge_sync::base::Result<void> start() override;
    edge_sync::base::Result<void> connect(
        const edge_sync::connection::Endpoint& endpoint) override;
    void async_send(const edge_sync::connection::ByteBuffer& data,
                    edge_sync::connection::CompletionHandler handler) override;
    void close() noexcept override;
};

}  // namespace edge_sync::transport
