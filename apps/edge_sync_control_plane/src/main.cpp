#include <drogon/drogon.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include "edge_sync_base/logger.hpp"
#include "edge_sync_control_plane/application/agent_gateway.hpp"
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

// Constant-time comparison to avoid timing side-channels on the token.
bool constant_time_eq(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

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
        if (std::string(argv[i]) == "--migrate-only")
            migrate_only = true;
    }
    const std::string db_url = std::getenv("DATABASE_URL")
                                   ? std::getenv("DATABASE_URL")
                                   : "host=localhost port=5432 dbname=edge_sync user=edge_sync "
                                     "password=edge_sync";

    Database db(db_url);
    bool connected = false;
    for (int i = 0; i < 30; ++i) {
        if (db.connect().ok()) {
            connected = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!connected) {
        LOG_ERROR << "could not connect to postgres after 30 retries";
        return 1;
    }
    const char* mig =
        std::getenv("MIGRATIONS_DIR") ? std::getenv("MIGRATIONS_DIR") : "/opt/edge-sync/migrations";
    if (auto r = db.migrate(mig); !r) {
        LOG_ERROR << "migrations failed: " << r.error().message;
        return 1;
    }
    if (migrate_only) {
        LOG_INFO << "migrations applied; exiting";
        return 0;
    }

    const std::string admin_token =
        std::getenv("EDGE_SYNC_ADMIN_TOKEN") ? std::getenv("EDGE_SYNC_ADMIN_TOKEN") : "";
    const auto authorized = [admin_token](const HttpRequestPtr& req) {
        if (admin_token.empty())
            return true;  // auth disabled when unconfigured
        const std::string auth = req->getHeader("Authorization");
        const std::string prefix = "Bearer ";
        if (auth.rfind(prefix, 0) != 0)
            return false;
        return constant_time_eq(auth.substr(prefix.size()), admin_token);
    };

    auto robot_service = std::make_shared<RobotService>(db);
    edge_sync::base::ConsoleLogger logger(edge_sync::base::LogLevel::Info);
    auto gateway = std::make_shared<edge_sync::control_plane::AgentGateway>(db, logger);

    const std::string fleet_secret =
        std::getenv("EDGE_SYNC_FLEET_SECRET") ? std::getenv("EDGE_SYNC_FLEET_SECRET") : "";
    if (auto r = gateway->start(7443, fleet_secret); !r) {
        LOG_WARN << "agent gateway failed to start: " << r.error().message;
    }
    auto sync_job_service = std::make_shared<SyncJobService>(db);

    app()
        .registerHandler(
            "/api/v1/health",
            [](const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& cb) {
                Json::Value json;
                json["status"] = "ok";
                cb(HttpResponse::newHttpJsonResponse(json));
            })
        .registerHandler(
            "/api/v1/robots",
            [robot_service](const HttpRequestPtr&,
                            std::function<void(const HttpResponsePtr&)>&& cb) {
                Json::Value arr(Json::arrayValue);
                auto robots = robot_service->list();
                if (!robots) {
                    cb(HttpResponse::newHttpJsonResponse(error_json(robots.error().message)));
                    return;
                }
                const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count();
                for (const auto& r : robots.value()) {
                    Json::Value j;
                    j["robotId"] = r.robot_id;
                    j["name"] = r.name;
                    const bool online = (now_ms - r.last_seen_ms) < 15000;
                    j["status"] = online ? "online" : "offline";
                    j["runtimeVersion"] = r.runtime_version;
                    j["lastSeenMs"] = r.last_seen_ms;
                    arr.append(j);
                }
                cb(HttpResponse::newHttpJsonResponse(arr));
            })
        .registerHandler(
            "/api/v1/sync-jobs",
            [sync_job_service, authorized](const HttpRequestPtr& req,
                                           std::function<void(const HttpResponsePtr&)>&& cb) {
                if (req->getMethod() == Post) {
                    if (!authorized(req)) {
                        auto resp = HttpResponse::newHttpJsonResponse(error_json("unauthorized"));
                        resp->setStatusCode(k401Unauthorized);
                        cb(resp);
                        return;
                    }
                    const auto body = req->getJsonObject();
                    if (!body) {
                        cb(HttpResponse::newHttpJsonResponse(error_json("invalid JSON body")));
                        return;
                    }
                    auto created =
                        sync_job_service->create((*body).get("source_robot", "").asString(),
                                                 (*body).get("target_robot", "").asString(),
                                                 (*body).get("artifact_id", "").asString());
                    if (!created) {
                        cb(HttpResponse::newHttpJsonResponse(error_json(created.error().message)));
                        return;
                    }
                    auto resp = HttpResponse::newHttpJsonResponse(job_json(created.value()));
                    resp->setStatusCode(k201Created);
                    cb(resp);
                    return;
                }
                auto jobs = sync_job_service->list();
                if (!jobs) {
                    cb(HttpResponse::newHttpJsonResponse(error_json(jobs.error().message)));
                    return;
                }
                Json::Value arr(Json::arrayValue);
                for (const auto& j : jobs.value()) arr.append(job_json(j));
                cb(HttpResponse::newHttpJsonResponse(arr));
            })
        .registerHandler(
            "/api/v1/sync-jobs/{job_id}",
            [sync_job_service](const HttpRequestPtr& req,
                               std::function<void(const HttpResponsePtr&)>&& cb) {
                const std::string path = req->getPath();
                const std::string prefix = "/api/v1/sync-jobs/";
                const std::string id =
                    path.size() > prefix.size() ? path.substr(prefix.size()) : "";
                auto job = sync_job_service->get(id);
                if (!job) {
                    auto resp = HttpResponse::newHttpJsonResponse(error_json(job.error().message));
                    resp->setStatusCode(k404NotFound);
                    cb(resp);
                    return;
                }
                cb(HttpResponse::newHttpJsonResponse(job_json(job.value())));
            });

    app().addListener("0.0.0.0", 8080).run();
    return 0;
}
