#pragma once

#include <vector>

#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::core {

enum class SyncOutcome { Identical, Partial, VersionConflict };

struct SyncPlan {
    SyncOutcome outcome{SyncOutcome::Identical};
    edge_sync::base::ArtifactId artifact_id;
    edge_sync::base::Version remote_version;
    std::vector<ChunkDescriptor> needed_chunks;   // unique chunk hashes to fetch
    std::uint64_t total_chunks{0};
};

}  // namespace edge_sync::core
