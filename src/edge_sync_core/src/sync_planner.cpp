#include "edge_sync_core/sync_planner.hpp"

#include <set>

#include "edge_sync_core/version.hpp"

namespace edge_sync::core {

edge_sync::base::Result<SyncPlan> SyncPlanner::plan(const std::optional<Manifest>& local,
                                                    const Manifest& remote) {
    SyncPlan plan;
    plan.artifact_id = remote.artifact_id;
    plan.remote_version = remote.version;
    plan.total_chunks = remote.chunks.size();

    if (local.has_value() && local->artifact_id != remote.artifact_id) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::InvalidArgument,
                                      "artifact id mismatch"};
    }

    // The remote version being older than what we already hold is a policy
    // conflict: never downgrade.
    if (local.has_value() &&
        compare_versions(local->version, remote.version) == VersionOrder::Newer) {
        plan.outcome = SyncOutcome::VersionConflict;
        return plan;
    }

    if (local.has_value() && local->artifact_hash == remote.artifact_hash) {
        plan.outcome = SyncOutcome::Identical;
        return plan;
    }

    std::set<edge_sync::base::Hash> present;
    if (local.has_value()) {
        for (const auto& c : local->chunks) present.insert(c.hash);
    }

    std::set<edge_sync::base::Hash> needed;
    for (const auto& c : remote.chunks) {
        if (present.count(c.hash)) continue;   // duplicate / already-present content
        if (needed.count(c.hash)) continue;    // dedup identical chunks
        needed.insert(c.hash);
        plan.needed_chunks.push_back(c);
    }

    plan.outcome = plan.needed_chunks.empty() ? SyncOutcome::Identical : SyncOutcome::Partial;
    return plan;
}

}  // namespace edge_sync::core
