#include "edge_sync_core/sync_engine.hpp"

#include <optional>
#include <vector>

#include "edge_sync_base/sha256.hpp"
#include "edge_sync_core/sync_planner.hpp"

namespace edge_sync::core {
namespace {

class VerifyingReceiver final : public IChunkReceiver {
public:
    VerifyingReceiver(IArtifactStore& store, const Manifest& manifest)
        : store_(store), manifest_(manifest) {}

    edge_sync::base::Result<void> on_chunk(std::uint64_t index,
                                           edge_sync::base::ByteBuffer data) override {
        const ChunkDescriptor* anchor = nullptr;
        for (const auto& c : manifest_.chunks) {
            if (c.index == index) {
                anchor = &c;
                break;
            }
        }
        if (anchor == nullptr) {
            return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                          "unexpected chunk index " + std::to_string(index)};
        }

        const edge_sync::base::Hash h = edge_sync::base::sha256_hex(data);
        if (h != anchor->hash) {
            return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                          "chunk " + std::to_string(index) + " hash mismatch"};
        }

        // Identical content may appear at multiple offsets; assemble it at each.
        for (const auto& c : manifest_.chunks) {
            if (c.hash == h) {
                if (auto r = store_.stage_chunk(manifest_.artifact_id, manifest_.version, c,
                                                data); !r) {
                    return r.error();
                }
            }
        }
        return {};
    }

private:
    IArtifactStore& store_;
    const Manifest& manifest_;
};

}  // namespace

SyncEngine::SyncEngine(IRemotePeer& peers, IArtifactRepository& repository, ITransfer& transfer,
                       IArtifactStore& store)
    : peers_(peers), repository_(repository), transfer_(transfer), store_(store) {}

edge_sync::base::Result<SyncOutcome> SyncEngine::synchronize(
    const edge_sync::base::RobotId& source, const edge_sync::base::ArtifactId& artifact_id) {
    auto remote = peers_.request_manifest(source, artifact_id);
    if (!remote) return remote.error();

    std::optional<Manifest> local;
    const auto exists = repository_.exists(artifact_id);
    if (exists.ok() && exists.value()) {
        const auto loaded = repository_.load(artifact_id);
        if (loaded.ok()) local = loaded.value();
    }

    SyncPlanner planner;
    auto plan = planner.plan(local, remote.value());
    if (!plan) return plan.error();
    if (plan.value().outcome == SyncOutcome::Identical) return SyncOutcome::Identical;
    if (plan.value().outcome == SyncOutcome::VersionConflict) {
        return SyncOutcome::VersionConflict;
    }

    std::vector<std::uint64_t> indices;
    for (const auto& c : plan.value().needed_chunks) indices.push_back(c.index);

    VerifyingReceiver receiver(store_, remote.value());
    if (auto r = transfer_.pull_chunks(source, artifact_id, indices, receiver); !r) {
        return r.error();
    }

    if (auto r = store_.commit(remote.value()); !r) return r.error();
    if (auto r = repository_.store(remote.value()); !r) return r.error();

    return SyncOutcome::Partial;  // transfer completed (was a partial diff)
}

}  // namespace edge_sync::core
