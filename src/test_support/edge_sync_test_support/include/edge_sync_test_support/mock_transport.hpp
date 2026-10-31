#pragma once

#include "edge_sync_connection/transport_interface.hpp"

namespace edge_sync::test_support {

class MockTransport final : public edge_sync::connection::ITransport {
public:
    edge_sync::base::Result<void> start() override { return {}; }
    edge_sync::base::Result<void> connect(
        const edge_sync::connection::Endpoint&) override { return {}; }
    void async_send(const edge_sync::connection::ByteBuffer&,
                    edge_sync::connection::CompletionHandler handler) override {
        handler(edge_sync::base::Result<void>{});
    }
    void close() noexcept override {}
};

}  // namespace edge_sync::test_support
