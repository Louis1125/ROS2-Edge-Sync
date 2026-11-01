#pragma once

#include <string>
#include <vector>

#include "edge_sync_base/result.hpp"
#include "edge_sync_control_plane/domain/robot.hpp"
#include "edge_sync_control_plane/domain/sync_job.hpp"

struct pg_conn;

namespace edge_sync::control_plane {

// Thin libpq wrapper. Runs schema migrations on connect.
class Database {
public:
    explicit Database(std::string connection_info);

    edge_sync::base::Result<void> connect();
    edge_sync::base::Result<void> migrate(const std::string& migrations_dir);
    void close();

    edge_sync::base::Result<std::vector<RobotRecord>> list_robots();
    edge_sync::base::Result<void> upsert_robot(const RobotRecord& robot);

    edge_sync::base::Result<SyncJob> create_sync_job(const SyncJob& job);
    edge_sync::base::Result<std::vector<SyncJob>> list_sync_jobs();
    edge_sync::base::Result<SyncJob> get_sync_job(const std::string& job_id);
    edge_sync::base::Result<void> update_sync_job_status(const std::string& job_id,
                                                         const std::string& status,
                                                         const std::string& error);

private:
    edge_sync::base::Result<void> execute(const std::string& sql);

    std::string connection_info_;
    pg_conn* conn_{nullptr};
};

}  // namespace edge_sync::control_plane
