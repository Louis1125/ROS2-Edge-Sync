#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <vector>

#include "edge_sync_base/result.hpp"
#include "edge_sync_control_plane/domain/sync_job.hpp"
#include "edge_sync_control_plane/infrastructure/database.hpp"

namespace edge_sync::control_plane {

// Application service for sync jobs. All interfaces (REST, CLI, SDK, MCP) call
// this service rather than touching the database directly.
class SyncJobService {
   public:
    explicit SyncJobService(Database& db) : db_(db) {}

    edge_sync::base::Result<SyncJob> create(const std::string& source, const std::string& target,
                                            const std::string& artifact_id) {
        const std::string job_id = make_job_id();
        // Transactional outbox: the job and its dispatch intent commit together.
        if (auto r = db_.enqueue_sync_job(job_id, job_id, source, target, artifact_id); !r) {
            return r.error();
        }
        SyncJob job;
        job.job_id = job_id;
        job.source_robot = source;
        job.target_robot = target;
        job.artifact_id = artifact_id;
        job.status = "queued";
        return job;
    }

    edge_sync::base::Result<std::vector<SyncJob>> list() {
        return db_.list_sync_jobs();
    }
    edge_sync::base::Result<SyncJob> get(const std::string& id) {
        return db_.get_sync_job(id);
    }

    edge_sync::base::Result<void> mark(const std::string& id, const std::string& status,
                                       const std::string& error = "") {
        return db_.update_sync_job_status(id, status, error);
    }

   private:
    static std::string make_job_id() {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
        return "sync-" + std::to_string(ms) + "-" + std::to_string(seq_++);
    }

    static std::atomic<std::uint64_t> seq_;
    Database& db_;
};

}  // namespace edge_sync::control_plane
