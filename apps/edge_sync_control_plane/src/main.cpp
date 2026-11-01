#include <drogon/drogon.h>

#include <atomic>
#include <string>

#include "edge_sync_control_plane/application/robot_service.hpp"
#include "edge_sync_control_plane/application/sync_job_service.hpp"
#include "edge_sync_control_plane/infrastructure/database.hpp"

using namespace drogon;
using edge_sync::control_plane::Database;
using edge_sync::control_plane::RobotService;
using edge_sync::control_plane::SyncJob;
using edge_sync::control_plane::SyncJobService;

namespace {
std::atomic<std::uint64_t> g_job_seq{0};

Json::Value error_json(const std::string& msg) {
    Json::Value v;
    v["error"] = msg;
    return v;
}

Json::Value job_json(const SyncJob& j) {
    Json::Value v;
    v["jobId"] = j.job_id;
    v["sourceRobot"] = j.source_robot;
    v["targetRobot"] = j.target_robot;
    v["artifact"] = j.artifact_id;
    v["status"] = j.status;
    v["transferredBytes"] = j.transferred_bytes;
    v["totalBytes"] = j.total_bytes;
    v["error"] = j.error;
    return v;
}
}  // namespace

int main(int argc, char** argv) {
    bool migrate_only = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--migrate-only") migrate_only = true;
    }
    const std::string db_url = std::getenv("DATABASE_URL")
                                   ? std::getenv("DATABASE_URL")
                                   : "host=localhost port=5432 dbname=edge_sync user=edge_sync "
                                     "password=edge_sync";

    Database db(db_url);
    if (auto r = db.connect(); !r) {
        LOG_ERROR << r.error().message;
        return 1;
    }
    const char* mig = std::getenv("MIGRATIONS_DIR") ? std::getenv("MIGRATIONS_DIR")
                                                    : "/opt/edge-sync/migrations";
    if (auto r = db.migrate(mig); !r) {
        LOG_ERROR << "migrations failed: " << r.error().message;
        return 1;
    }
    if (migrate_only) {
        LOG_INFO << "migrations applied; exiting";
        return 0;
    }

    auto robot_service = std::make_shared<RobotService>(db);
    auto sync_job_service = std::make_shared<SyncJobService>(db);

    app()
        .registerHandler("/api/v1/health",
                         [](const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& cb) {
                             Json::Value json;
                             json["status"] = "ok";
                             cb(HttpResponse::newHttpJsonResponse(json));
                         })
        .registerHandler("/api/v1/robots",
                         [robot_service](const HttpRequestPtr&,
                                         std::function<void(const HttpResponsePtr&)>&& cb) {
                             Json::Value arr(Json::arrayValue);
                             auto robots = robot_service->list();
                             if (!robots) {
                                 cb(HttpResponse::newHttpJsonResponse(
                                     error_json(robots.error().message)));
                                 return;
                             }
                             for (const auto& r : robots.value()) {
                                 Json::Value j;
                                 j["robotId"] = r.robot_id;
                                 j["name"] = r.name;
                                 j["status"] = r.status;
                                 j["runtimeVersion"] = r.runtime_version;
                                 j["lastSeenMs"] = r.last_seen_ms;
                                 arr.append(j);
                             }
                             cb(HttpResponse::newHttpJsonResponse(arr));
                         })
        .registerHandler("/api/v1/sync-jobs",
                         [sync_job_service](const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& cb) {
                             if (req->getMethod() == Post) {
                                 const auto body = req->getJsonObject();
                                 if (!body) {
                                     cb(HttpResponse::newHttpJsonResponse(
                                         error_json("invalid JSON body")));
                                     return;
                                 }
                                 auto created = sync_job_service->create(
                                     (*body).get("source_robot", "").asString(),
                                     (*body).get("target_robot", "").asString(),
                                     (*body).get("artifact_id", "").asString());
                                 if (!created) {
                                     cb(HttpResponse::newHttpJsonResponse(
                                         error_json(created.error().message)));
                                     return;
                                 }
                                 auto resp = HttpResponse::newHttpJsonResponse(job_json(created.value()));
                                 resp->setStatusCode(k201Created);
                                 cb(resp);
                                 return;
                             }
                             auto jobs = sync_job_service->list();
                             if (!jobs) {
                                 cb(HttpResponse::newHttpJsonResponse(
                                     error_json(jobs.error().message)));
                                 return;
                             }
                             Json::Value arr(Json::arrayValue);
                             for (const auto& j : jobs.value()) arr.append(job_json(j));
                             cb(HttpResponse::newHttpJsonResponse(arr));
                         })
        .registerHandler("/api/v1/sync-jobs/{job_id}",
                         [sync_job_service](const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& cb) {
                             const std::string path = req->getPath();
                             const std::string prefix = "/api/v1/sync-jobs/";
                             const std::string id =
                                 path.size() > prefix.size() ? path.substr(prefix.size()) : "";
                             auto job = sync_job_service->get(id);
                             if (!job) {
                                 auto resp = HttpResponse::newHttpJsonResponse(
                                     error_json(job.error().message));
                                 resp->setStatusCode(k404NotFound);
                                 cb(resp);
                                 return;
                             }
                             cb(HttpResponse::newHttpJsonResponse(job_json(job.value())));
                         });

    app().addListener("0.0.0.0", 8080).run();
    return 0;
}
