#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_core/remote_peer.hpp"
#include "edge_sync_core/repository_interfaces.hpp"
#include "edge_sync_core/transfer.hpp"

namespace edge_sync::connection {

// Data-plane adapter: implements both core::IRemotePeer (manifest) and
// core::ITransfer (chunk streaming) over the ConnectionManager. Bulk chunk data
// is receiver-driven and pipelined (bounded sliding window); the I/O thread
// only copies bytes, and disk I/O happens on the caller/worker threads.
class SyncPeerAdapter final : public edge_sync::core::IRemotePeer,
                              public edge_sync::core::ITransfer {
public:
    SyncPeerAdapter(ConnectionManager& connections,
                    edge_sync::core::IArtifactRepository& local_repository,
                    edge_sync::core::IChunkProvider& chunks);
    ~SyncPeerAdapter() override;

    edge_sync::base::Result<edge_sync::core::Manifest> request_manifest(
        const edge_sync::base::RobotId& robot_id,
        const edge_sync::base::ArtifactId& artifact_id) override;

    edge_sync::base::Result<void> pull_chunks(
        const edge_sync::base::RobotId& source, const edge_sync::base::ArtifactId& artifact_id,
        const std::vector<std::uint64_t>& chunk_indices,
        edge_sync::core::IChunkReceiver& receiver) override;

    void on_application_message(const Peer& peer,
                                const edge_sync::protocol::MessageEnvelope& env);
    void stop();

private:
    struct PullContext {
        std::mutex mu;
        std::condition_variable cv;
        std::map<std::uint64_t, ByteBuffer> data;
        bool failed{false};
        std::string error;
    };
    struct ServeTask {
        std::string target_robot;
        std::string transfer_id;
        std::string artifact_id;
        std::uint64_t chunk_index{0};
    };

    void send_chunk_request(const std::string& robot, const std::string& transfer_id,
                            const std::string& artifact_id, std::uint64_t chunk_index);
    void send_chunk_ack(const std::string& robot, const std::string& transfer_id,
                        std::uint64_t chunk_index);
    void worker_loop(std::stop_token stop);

    ConnectionManager& connections_;
    edge_sync::core::IArtifactRepository& local_repository_;
    edge_sync::core::IChunkProvider& chunks_;

    std::atomic<std::uint64_t> transfer_seq_{1};

    std::mutex pulls_mu_;
    std::map<std::string, std::shared_ptr<PullContext>> pulls_;

    std::mutex queue_mu_;
    std::condition_variable_any queue_cv_;
    std::queue<ServeTask> queue_;
    std::jthread worker_;
    std::atomic<bool> stopped_{false};
};

}  // namespace edge_sync::connection
