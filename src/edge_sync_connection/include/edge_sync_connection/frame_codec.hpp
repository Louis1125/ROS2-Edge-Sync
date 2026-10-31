#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "edge_sync_base/types.hpp"

namespace edge_sync::connection {

using ByteBuffer = edge_sync::base::ByteBuffer;

// Length-prefixed framing: [u32 big-endian length][payload].
ByteBuffer encode_frame(const ByteBuffer& payload);

// Incremental decoder. Feed raw bytes as they arrive; complete frames are
// appended to `out` in arrival order. Survives partial/multiple frames per feed.
class FrameDecoder {
public:
    void feed(const std::uint8_t* data, std::size_t size, std::vector<ByteBuffer>& out);
    void reset() { buffer_.clear(); }

private:
    ByteBuffer buffer_;
};

}  // namespace edge_sync::connection
