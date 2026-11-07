#pragma once

#include <cstdint>
#include <vector>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::core {

// Provides chunk bytes by content hash (implemented by storage). The sender
// reads chunks to stream them.
class IChunkProvider {
   public:
    virtual ~IChunkProvider() = default;
    virtual edge_sync::base::Result<edge_sync::base::ByteBuffer> get_chunk(
        const edge_sync::base::Hash& hash) = 0;
};

// Receives streamed chunks on the target side. Implemented by the sync engine:
// verify the chunk hash, write to staging and persist metadata. Returning an
// Error aborts the transfer and prevents the chunk from being acknowledged.
class IChunkReceiver {
   public:
    virtual ~IChunkReceiver() = default;
    virtual edge_sync::base::Result<void> on_chunk(std::uint64_t index,
                                                   edge_sync::base::ByteBuffer data) = 0;
};

// Pull-based chunk transfer port. Implemented by the connection layer over the
// ConnectionManager. The caller (sync engine) drives; the transport streams
// with a bounded sliding window and never blocks the I/O thread.
class ITransfer {
   public:
    virtual ~ITransfer() = default;
    virtual edge_sync::base::Result<void> pull_chunks(
        const edge_sync::base::RobotId& source, const edge_sync::base::ArtifactId& artifact_id,
        const std::vector<std::uint64_t>& chunk_indices, IChunkReceiver& receiver) = 0;
};

}  // namespace edge_sync::core
