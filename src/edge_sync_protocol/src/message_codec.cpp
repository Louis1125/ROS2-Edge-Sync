#include "edge_sync_protocol/message_codec.hpp"

#include <cstdint>

namespace edge_sync::protocol {

edge_sync::base::Result<edge_sync::base::ByteBuffer> serialize(const MessageEnvelope& envelope) {
    edge_sync::base::ByteBuffer buffer(static_cast<std::size_t>(envelope.ByteSizeLong()));
    if (!envelope.SerializeToArray(buffer.data(), static_cast<int>(buffer.size()))) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ProtocolMismatch,
                                      "failed to serialize envelope"};
    }
    return buffer;
}

edge_sync::base::Result<MessageEnvelope> parse(const edge_sync::base::ByteBuffer& bytes) {
    MessageEnvelope envelope;
    if (!envelope.ParseFromArray(bytes.data(), static_cast<int>(bytes.size()))) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ProtocolMismatch,
                                      "failed to parse envelope"};
    }
    return envelope;
}

}  // namespace edge_sync::protocol
