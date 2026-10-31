#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "edge_sync_base/config.hpp"
#include "edge_sync_base/logger.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/tcp_transport.hpp"
#include "edge_sync_storage/artifact_repository.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

using namespace edge_sync;

namespace {

std::atomic<bool> g_stop{false};
void on_signal(int) { g_stop = true; }

base::LogLevel parse_level(const std::string& s) {
    if (s == "trace") return base::LogLevel::Trace;
    if (s == "debug") return base::LogLevel::Debug;
    if (s == "warn") return base::LogLevel::Warn;
    if (s == "error") return base::LogLevel::Error;
    return base::LogLevel::Info;
}

std::vector<connection::Endpoint> parse_peers(const std::string& spec) {
    std::vector<connection::Endpoint> peers;
    if (spec.empty()) return peers;
    std::size_t start = 0;
    while (start <= spec.size()) {
        const auto comma = spec.find(',', start);
        const auto token = spec.substr(start, comma == std::string::npos ? std::string::npos
                                                                         : comma - start);
        const auto colon = token.rfind(':');
        if (colon != std::string::npos) {
            peers.push_back({token.substr(0, colon),
                             static_cast<std::uint16_t>(std::stoi(token.substr(colon + 1)))});
        }
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return peers;
}

}  // namespace

int main(int argc, char** argv) {
    std::string config_path = "config/robot.yaml";
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) config_path = argv[++i];
    }

    auto cfg = base::Config::load_file(config_path);
    if (!cfg) {
        std::fprintf(stderr, "config error: %s\n", cfg.error().message.c_str());
        return 1;
    }
    const auto& c = cfg.value();

    base::ConsoleLogger logger(parse_level(c.get_string("logging.level", "info")));
    const std::string robot_id = c.get_string("robot.id", "robot-0");

    // --- storage ---
    const std::string storage_root = c.get_string("storage.root", "/var/lib/edge-sync");
    const std::string db_path =
        storage_root + "/" + c.get_string("storage.database", "metadata.sqlite");
    storage::SqliteRepository sqlite(db_path);
    if (auto r = sqlite.open(); !r) {
        logger.log(base::LogLevel::Error, "sqlite: " + r.error().message);
        return 1;
    }
    storage::ObjectStore objects(storage_root + "/objects");
    if (auto r = objects.initialize(); !r) {
        logger.log(base::LogLevel::Error, "object store: " + r.error().message);
        return 1;
    }
    storage::ArtifactRepository repository(sqlite, objects);
    (void)repository;  // wired into the sync engine in M4

    // --- connection ---
    const std::string listen = c.get_string("connection.listen", "0.0.0.0");
    const std::uint16_t port = static_cast<std::uint16_t>(c.get_int("connection.port", 7443));

    auto transport = std::make_unique<connection::TcpTransport>();
    connection::ConnectionManager manager(std::move(transport), logger);
    manager.set_local_identity(robot_id);

    if (auto r = manager.start({listen, port}); !r) {
        logger.log(base::LogLevel::Error, "listen failed: " + r.error().message);
        return 1;
    }
    logger.log(base::LogLevel::Info,
               "robot '" + robot_id + "' listening on " + listen + ":" + std::to_string(port));

    for (const auto& peer : parse_peers(c.get_string("connection.peers", ""))) {
        if (auto r = manager.connect_peer(peer); !r) {
            logger.log(base::LogLevel::Warn,
                       "connect to " + peer.address + ":" + std::to_string(peer.port) +
                           " failed: " + r.error().message);
        }
    }

    logger.log(base::LogLevel::Info, "runtime ready; press Ctrl-C to stop");
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    logger.log(base::LogLevel::Info, "shutting down");
    manager.stop();
    if (auto r = sqlite.close(); !r) {
        logger.log(base::LogLevel::Error, "sqlite close: " + r.error().message);
    }
    logger.log(base::LogLevel::Info, "bye");
    return 0;
}
