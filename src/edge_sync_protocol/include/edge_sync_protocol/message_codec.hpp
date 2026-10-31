#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "v1/sync.pb.h"

namespace edge_sync::protocol {

inline constexpr std::uint32_t kProtocolVersion = 1;

// Re-export the generated wire type under the public namespace so consumers
// use edge_sync::protocol::MessageEnvelope rather than the versioned v1 type.
using MessageEnvelope = v1::MessageEnvelope;

// Serialize an envelope to a byte buffer for framing/transport.
edge_sync::base::Result<edge_sync::base::ByteBuffer> serialize(const MessageEnvelope& envelope);

// Parse an envelope from raw bytes. Fails with ProtocolMismatch on malformed data.
edge_sync::base::Result<MessageEnvelope> parse(const edge_sync::base::ByteBuffer& bytes);

}  // namespace edge_sync::protocol
