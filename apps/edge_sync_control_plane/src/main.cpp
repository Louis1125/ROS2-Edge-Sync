#include <drogon/drogon.h>

#include <string>

#include "edge_sync_control_plane/application/robot_service.hpp"
#include "edge_sync_control_plane/infrastructure/database.hpp"

using namespace drogon;
using edge_sync::control_plane::Database;
using edge_sync::control_plane::RobotService;

int main(int argc, char** argv) {
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

    auto robot_service = std::make_shared<RobotService>(db);

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
                                 Json::Value err;
                                 err["error"] = robots.error().message;
                                 cb(HttpResponse::newHttpJsonResponse(err));
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
                         });

    app().addListener("0.0.0.0", 8080).run();
    return 0;
}
