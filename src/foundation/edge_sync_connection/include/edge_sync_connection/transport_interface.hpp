#pragma once

#include <functional>

#include "edge_sync_base/result.hpp"
#include "edge_sync_connection/endpoint.hpp"

namespace edge_sync::connection {

using ByteBuffer = edge_sync::base::ByteBuffer;
using CompletionHandler = std::function<void(edge_sync::base::Result<void>)>;

class ITransport {
public:
    virtual ~ITransport() = default;

    virtual edge_sync::base::Result<void> start() = 0;
    virtual edge_sync::base::Result<void> connect(const Endpoint& endpoint) = 0;
    virtual void async_send(const ByteBuffer& data, CompletionHandler handler) = 0;
    virtual void close() noexcept = 0;
};

}  // namespace edge_sync::connection
