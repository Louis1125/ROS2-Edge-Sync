#pragma once

#include <optional>

#include "edge_sync_base/result.hpp"
#include "edge_sync_core/manifest.hpp"
#include "edge_sync_core/sync_plan.hpp"

namespace edge_sync::core {

// Compares a local manifest (if any) against a remote one and decides which
// chunks must be fetched. Chunk comparison is by content hash, so duplicate
// content already present locally is never re-transferred.
class SyncPlanner {
public:
    edge_sync::base::Result<SyncPlan> plan(const std::optional<Manifest>& local,
                                           const Manifest& remote);
};

}  // namespace edge_sync::core
