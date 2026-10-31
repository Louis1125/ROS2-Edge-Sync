#include "edge_sync_connection/frame_codec.hpp"

#include <cstring>

namespace edge_sync::connection {

ByteBuffer encode_frame(const ByteBuffer& payload) {
    ByteBuffer frame(4 + payload.size());
    const std::uint32_t len = static_cast<std::uint32_t>(payload.size());
    frame[0] = static_cast<std::uint8_t>((len >> 24) & 0xff);
    frame[1] = static_cast<std::uint8_t>((len >> 16) & 0xff);
    frame[2] = static_cast<std::uint8_t>((len >> 8) & 0xff);
    frame[3] = static_cast<std::uint8_t>(len & 0xff);
    std::memcpy(frame.data() + 4, payload.data(), payload.size());
    return frame;
}

void FrameDecoder::feed(const std::uint8_t* data, std::size_t size,
                        std::vector<ByteBuffer>& out) {
    buffer_.insert(buffer_.end(), data, data + size);
    while (buffer_.size() >= 4) {
        const std::uint32_t len =
            (static_cast<std::uint32_t>(buffer_[0]) << 24) |
            (static_cast<std::uint32_t>(buffer_[1]) << 16) |
            (static_cast<std::uint32_t>(buffer_[2]) << 8) |
            static_cast<std::uint32_t>(buffer_[3]);
        if (buffer_.size() < 4 + len) break;
        out.emplace_back(buffer_.begin() + 4, buffer_.begin() + 4 + len);
        buffer_.erase(buffer_.begin(), buffer_.begin() + 4 + len);
    }
}

}  // namespace edge_sync::connection
