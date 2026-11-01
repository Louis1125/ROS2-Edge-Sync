#pragma once

#include <vector>

#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::core {

struct SyncPlan {
    edge_sync::base::ArtifactId artifact_id;
    edge_sync::base::Version remote_version;
    std::vector<FileEntry> needed_files;
};

}  // namespace edge_sync::core
