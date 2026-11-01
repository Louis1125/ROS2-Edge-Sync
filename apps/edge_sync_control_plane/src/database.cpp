#include "edge_sync_control_plane/infrastructure/database.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <libpq-fe.h>

namespace edge_sync::control_plane {

Database::Database(std::string connection_info) : connection_info_(std::move(connection_info)) {}

edge_sync::base::Result<void> Database::connect() {
    conn_ = PQconnectdb(connection_info_.c_str());
    if (PQstatus(conn_) != CONNECTION_OK) {
        const auto msg = std::string(PQerrorMessage(conn_));
        PQfinish(conn_);
        conn_ = nullptr;
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "postgres connect: " + msg};
    }
    return {};
}

void Database::close() {
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

edge_sync::base::Result<void> Database::execute(const std::string& sql) {
    PGresult* res = PQexec(conn_, sql.c_str());
    const bool ok = PQresultStatus(res) == PGRES_COMMAND_OK ||
                    PQresultStatus(res) == PGRES_TUPLES_OK;
    const auto err = std::string(PQerrorMessage(conn_));
    PQclear(res);
    if (!ok) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "postgres: " + err};
    }
    return {};
}

edge_sync::base::Result<void> Database::migrate(const std::string& migrations_dir) {
    std::vector<std::filesystem::path> files;
    for (const auto& e : std::filesystem::directory_iterator(migrations_dir)) {
        if (e.path().extension() == ".sql") files.push_back(e.path());
    }
    std::sort(files.begin(), files.end());
    for (const auto& f : files) {
        std::ifstream in(f);
        std::stringstream ss;
        ss << in.rdbuf();
        if (auto r = execute(ss.str()); !r) return r.error();
    }
    return {};
}

edge_sync::base::Result<std::vector<RobotRecord>> Database::list_robots() {
    PGresult* res = PQexec(conn_, "SELECT robot_id, name, status, runtime_version, last_seen_ms "
                                 "FROM robots ORDER BY robot_id");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        const auto err = std::string(PQerrorMessage(conn_));
        PQclear(res);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure, "postgres: " + err};
    }
    std::vector<RobotRecord> robots;
    const int n = PQntuples(res);
    for (int i = 0; i < n; ++i) {
        RobotRecord r;
        r.robot_id = PQgetvalue(res, i, 0);
        r.name = PQgetvalue(res, i, 1);
        r.status = PQgetvalue(res, i, 2);
        r.runtime_version = PQgetvalue(res, i, 3);
        r.last_seen_ms = std::stoll(PQgetvalue(res, i, 4));
        robots.push_back(std::move(r));
    }
    PQclear(res);
    return robots;
}

edge_sync::base::Result<void> Database::upsert_robot(const RobotRecord& robot) {
    std::string sql = "INSERT INTO robots(robot_id, name, status, runtime_version, last_seen_ms) "
                      "VALUES ('" + robot.robot_id + "', '" + robot.name + "', '" + robot.status +
                      "', '" + robot.runtime_version + "', " + std::to_string(robot.last_seen_ms) +
                      ") ON CONFLICT(robot_id) DO UPDATE SET status=EXCLUDED.status, "
                      "last_seen_ms=EXCLUDED.last_seen_ms";
    return execute(sql);
}

edge_sync::base::Result<SyncJob> Database::create_sync_job(const SyncJob& job) {
    std::string sql =
        "INSERT INTO sync_jobs(job_id, source_robot, target_robot, artifact_id, status, "
        "transferred_bytes, total_bytes, error) VALUES ('" +
        job.job_id + "', '" + job.source_robot + "', '" + job.target_robot + "', '" +
        job.artifact_id + "', '" + job.status + "', 0, 0, '')";
    if (auto r = execute(sql); !r) return r.error();
    return get_sync_job(job.job_id);
}

edge_sync::base::Result<std::vector<SyncJob>> Database::list_sync_jobs() {
    PGresult* res = PQexec(conn_, "SELECT job_id, source_robot, target_robot, artifact_id, "
                                 "status, transferred_bytes, total_bytes, error FROM sync_jobs "
                                 "ORDER BY job_id");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        const auto err = std::string(PQerrorMessage(conn_));
        PQclear(res);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure, "postgres: " + err};
    }
    std::vector<SyncJob> jobs;
    for (int i = 0; i < PQntuples(res); ++i) {
        SyncJob j;
        j.job_id = PQgetvalue(res, i, 0);
        j.source_robot = PQgetvalue(res, i, 1);
        j.target_robot = PQgetvalue(res, i, 2);
        j.artifact_id = PQgetvalue(res, i, 3);
        j.status = PQgetvalue(res, i, 4);
        j.transferred_bytes = std::stoull(PQgetvalue(res, i, 5));
        j.total_bytes = std::stoull(PQgetvalue(res, i, 6));
        j.error = PQgetvalue(res, i, 7);
        jobs.push_back(std::move(j));
    }
    PQclear(res);
    return jobs;
}

edge_sync::base::Result<SyncJob> Database::get_sync_job(const std::string& job_id) {
    std::string sql = "SELECT job_id, source_robot, target_robot, artifact_id, status, "
                      "transferred_bytes, total_bytes, error FROM sync_jobs WHERE job_id = '" +
                      job_id + "'";
    PGresult* res = PQexec(conn_, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "job not found: " + job_id};
    }
    SyncJob j;
    j.job_id = PQgetvalue(res, 0, 0);
    j.source_robot = PQgetvalue(res, 0, 1);
    j.target_robot = PQgetvalue(res, 0, 2);
    j.artifact_id = PQgetvalue(res, 0, 3);
    j.status = PQgetvalue(res, 0, 4);
    j.transferred_bytes = std::stoull(PQgetvalue(res, 0, 5));
    j.total_bytes = std::stoull(PQgetvalue(res, 0, 6));
    j.error = PQgetvalue(res, 0, 7);
    PQclear(res);
    return j;
}

edge_sync::base::Result<void> Database::update_sync_job_status(const std::string& job_id,
                                                               const std::string& status,
                                                               const std::string& error) {
    std::string sql = "UPDATE sync_jobs SET status = '" + status + "', error = '" + error +
                      "' WHERE job_id = '" + job_id + "'";
    return execute(sql);
}

edge_sync::base::Result<void> Database::enqueue_sync_job(
    const std::string& job_id, const std::string& command_id, const std::string& source_robot,
    const std::string& target_robot, const std::string& artifact_id) {
    // Single transaction: the job and its outbox event commit together.
    if (auto r = execute("BEGIN"); !r) return r.error();
    const std::vector<std::string> p1{job_id, source_robot, target_robot, artifact_id,
                                      command_id, "pending"};
    const auto now = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    // Use PQexecParams for user-controlled values (injection-safe).
    {
        const char* sql =
            "INSERT INTO sync_jobs(job_id, source_robot, target_robot, artifact_id, status, "
            "command_id, delivery_state, transferred_bytes, total_bytes, error, updated_at) "
            "VALUES($1,$2,$3,$4,'queued',$5,'pending',0,0,'',$6)";
        const char* vals[6] = {p1[0].c_str(), p1[1].c_str(), p1[2].c_str(), p1[3].c_str(),
                               p1[4].c_str(), now.c_str()};
        PGresult* res = PQexecParams(conn_, sql, 6, nullptr, vals, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            const auto err = std::string(PQerrorMessage(conn_));
            PQclear(res);
            execute("ROLLBACK");
            return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                          "postgres: " + err};
        }
        PQclear(res);
    }
    {
        const char* sql =
            "INSERT INTO outbox(command_id, target_robot, source_robot, artifact_id, "
            "delivery_state, attempts, created_at) VALUES($1,$2,$3,$4,'pending',0,$5)";
        const char* vals[5] = {command_id.c_str(), target_robot.c_str(), source_robot.c_str(),
                               artifact_id.c_str(), now.c_str()};
        PGresult* res = PQexecParams(conn_, sql, 5, nullptr, vals, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            const auto err = std::string(PQerrorMessage(conn_));
            PQclear(res);
            execute("ROLLBACK");
            return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                          "postgres: " + err};
        }
        PQclear(res);
    }
    return execute("COMMIT");
}

edge_sync::base::Result<std::vector<Database::OutboxEntry>> Database::poll_outbox() {
    PGresult* res = PQexec(conn_,
                           "SELECT command_id, target_robot, source_robot, artifact_id "
                           "FROM outbox WHERE delivery_state IN ('pending','dispatched') "
                           "ORDER BY id");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        const auto err = std::string(PQerrorMessage(conn_));
        PQclear(res);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "postgres: " + err};
    }
    std::vector<OutboxEntry> out;
    for (int i = 0; i < PQntuples(res); ++i) {
        out.push_back({PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), PQgetvalue(res, i, 2),
                       PQgetvalue(res, i, 3)});
    }
    PQclear(res);
    return out;
}

edge_sync::base::Result<void> Database::mark_outbox(const std::string& command_id,
                                                    const std::string& delivery_state) {
    const char* sql = "UPDATE outbox SET delivery_state=$1, attempts=attempts+1 WHERE command_id=$2";
    const char* vals[2] = {delivery_state.c_str(), command_id.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 2, nullptr, vals, nullptr, nullptr, 0);
    const bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    const auto err = std::string(PQerrorMessage(conn_));
    PQclear(res);
    if (!ok) return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                           "postgres: " + err};
    return {};
}

edge_sync::base::Result<void> Database::update_job_execution(const std::string& command_id,
                                                             const std::string& status,
                                                             const std::string& error) {
    const char* sql =
        "UPDATE sync_jobs SET status=$1, error=$2, delivery_state='acknowledged' "
        "WHERE command_id=$3";
    const char* vals[3] = {status.c_str(), error.c_str(), command_id.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 3, nullptr, vals, nullptr, nullptr, 0);
    const bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    const auto err = std::string(PQerrorMessage(conn_));
    PQclear(res);
    if (!ok) return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                           "postgres: " + err};
    return {};
}

edge_sync::base::Result<void> Database::upsert_agent_session(const std::string& robot_id) {
    const char* sql =
        "INSERT INTO agent_sessions(robot_id, connected_at, last_seen_at) VALUES($1,$2,$2) "
        "ON CONFLICT(robot_id) DO UPDATE SET last_seen_at=$2";
    const auto now = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    const char* vals[2] = {robot_id.c_str(), now.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 2, nullptr, vals, nullptr, nullptr, 0);
    const bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    const auto err = std::string(PQerrorMessage(conn_));
    PQclear(res);
    if (!ok) return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                           "postgres: " + err};
    return {};
}

edge_sync::base::Result<void> Database::touch_robot(const std::string& robot_id) {
    const char* sql =
        "INSERT INTO robots(robot_id, name, status, runtime_version, last_seen_ms) "
        "VALUES($1, '', 'online', '', $2) "
        "ON CONFLICT(robot_id) DO UPDATE SET last_seen_ms=$2, status='online'";
    const auto now = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    const char* vals[2] = {robot_id.c_str(), now.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 2, nullptr, vals, nullptr, nullptr, 0);
    const bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    const auto err = std::string(PQerrorMessage(conn_));
    PQclear(res);
    if (!ok) return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                           "postgres: " + err};
    return {};
}

}  // namespace edge_sync::control_plane
