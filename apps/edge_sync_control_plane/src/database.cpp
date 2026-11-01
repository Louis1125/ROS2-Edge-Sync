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

}  // namespace edge_sync::control_plane
