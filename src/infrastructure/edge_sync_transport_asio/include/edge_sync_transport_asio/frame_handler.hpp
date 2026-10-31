#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::transport {

class FrameHandler {
public:
    edge_sync::base::Result<edge_sync::base::ByteBuffer> encode(
        const edge_sync::base::ByteBuffer& payload);
    edge_sync::base::Result<edge_sync::base::ByteBuffer> decode(
        const edge_sync::base::ByteBuffer& frame);
};

}  // namespace edge_sync::transport
