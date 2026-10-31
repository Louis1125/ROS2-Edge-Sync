#pragma once

#include <vector>

#include "edge_sync_base/result.hpp"
#include "edge_sync_domain/manifest.hpp"

namespace edge_sync::application {

class SyncPlanner {
public:
    edge_sync::base::Result<std::vector<edge_sync::domain::FileEntry>> plan(
        const edge_sync::domain::Manifest& local,
        const edge_sync::domain::Manifest& remote);
};

}  // namespace edge_sync::application
