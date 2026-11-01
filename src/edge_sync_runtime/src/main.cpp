#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "edge_sync_base/config.hpp"
#include "edge_sync_base/logger.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/sync_peer_adapter.hpp"
#include "edge_sync_connection/tcp_transport.hpp"
#include "edge_sync_core/manifest_builder.hpp"
#include "edge_sync_core/sync_engine.hpp"
#include "edge_sync_storage/artifact_repository.hpp"
#include "edge_sync_storage/artifact_store.hpp"
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
    const std::string fleet_secret = c.get_string("fleet.secret", "");

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
    storage::ArtifactStore artifact_store(storage_root + "/artifacts", objects, sqlite);
    if (auto r = artifact_store.initialize(); !r) {
        logger.log(base::LogLevel::Error, "artifact store: " + r.error().message);
        return 1;
    }

    // --seed <file> [--artifact <id>]: publish an artifact then exit.
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--seed" && i + 1 < argc) {
            const std::string file = argv[++i];
            std::string artifact_id = "artifact";
            for (int j = 1; j < argc; ++j) {
                if (std::string(argv[j]) == "--artifact" && j + 1 < argc) artifact_id = argv[++j];
            }
            std::ifstream in(file, std::ios::binary);
            base::ByteBuffer data((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
            auto manifest = core::build_manifest(artifact_id, {1, 0, 0}, data, 65536);
            if (!manifest) {
                std::fprintf(stderr, "seed: %s\n", manifest.error().message.c_str());
                return 1;
            }
            for (const auto& c : manifest.value().chunks) {
                base::ByteBuffer chunk(data.begin() + static_cast<std::ptrdiff_t>(c.offset),
                                       data.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
                if (auto r = objects.put(chunk); !r) {
                    std::fprintf(stderr, "seed: %s\n", r.error().message.c_str());
                    return 1;
                }
            }
            if (auto r = repository.store(manifest.value()); !r) {
                std::fprintf(stderr, "seed: %s\n", r.error().message.c_str());
                return 1;
            }
            std::printf("seeded artifact=%s version=1.0.0 hash=%s chunks=%zu\n",
                        artifact_id.c_str(), manifest.value().artifact_hash.c_str(),
                        manifest.value().chunks.size());
            return 0;
        }
    }

    // --- data plane (peer) connection + sync engine ---
    const std::string listen = c.get_string("connection.listen", "0.0.0.0");
    const std::uint16_t port = static_cast<std::uint16_t>(c.get_int("connection.port", 7443));

    auto peer_transport = std::make_unique<connection::TcpTransport>();
    connection::ConnectionManager peers(std::move(peer_transport), logger);
    peers.set_local_identity(robot_id);
    if (!fleet_secret.empty()) peers.set_shared_secret(fleet_secret);
    if (auto r = peers.start({listen, port}); !r) {
        logger.log(base::LogLevel::Error, "listen failed: " + r.error().message);
        return 1;
    }
    connection::SyncPeerAdapter adapter(peers, repository, objects);
    core::SyncEngine engine(adapter, repository, adapter, artifact_store);

    for (const auto& peer : parse_peers(c.get_string("connection.peers", ""))) {
        peers.connect_peer(peer);
    }

    // --- control plane (agent) connection ---
    std::unique_ptr<connection::ConnectionManager> control_plane;
    std::mutex executed_mu;
    std::set<std::string> executed;
    std::mutex workers_mu;
    std::vector<std::jthread> workers;

    const std::string cp_host = c.get_string("control_plane.host", "");
    std::jthread heartbeat_thread;
    if (!cp_host.empty()) {
        const std::uint16_t cp_port = static_cast<std::uint16_t>(c.get_int("control_plane.port", 7443));
        control_plane = std::make_unique<connection::ConnectionManager>(
            std::make_unique<connection::TcpTransport>(), logger);
        control_plane->set_local_identity(robot_id);
        if (!fleet_secret.empty()) control_plane->set_shared_secret(fleet_secret);
        control_plane->set_application_message_handler(
            [&](const connection::Peer& cp, const protocol::MessageEnvelope& env) {
                if (!env.has_sync_command()) return;
                const auto command_id = env.sync_command().command_id();
                const auto source = env.sync_command().source_robot();
                const auto artifact = env.sync_command().artifact_id();

                // idempotency: acknowledge and skip already-executed commands
                {
                    std::lock_guard lk(executed_mu);
                    if (executed.count(command_id)) return;
                }

                // acknowledge delivery
                protocol::MessageEnvelope ack;
                ack.set_protocol_version(protocol::kProtocolVersion);
                ack.mutable_sync_status_report()->set_command_id(command_id);
                ack.mutable_sync_status_report()->set_status("acknowledged");
                control_plane->send(cp.identity.robot_id, ack);

                // execute on a worker thread (do not block the I/O thread)
                workers.emplace_back([&, command_id, source, artifact]() {
                    auto outcome = engine.synchronize(source, artifact);
                    protocol::MessageEnvelope report;
                    report.set_protocol_version(protocol::kProtocolVersion);
                    auto* st = report.mutable_sync_status_report();
                    st->set_command_id(command_id);
                    if (outcome.ok()) {
                        st->set_status("succeeded");
                    } else {
                        st->set_status("failed");
                        st->set_error(outcome.error().message);
                    }
                    control_plane->send(cp.identity.robot_id, report);
                    std::lock_guard lk(executed_mu);
                    executed.insert(command_id);
                });
            });
        control_plane->start({"0.0.0.0", 0});
        control_plane->connect_peer({cp_host, cp_port});
        logger.log(base::LogLevel::Info,
                   "agent " + robot_id + " connecting to control plane " + cp_host);

        heartbeat_thread = std::jthread([&](std::stop_token stop) {
            while (!stop.stop_requested() && !g_stop) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                if (control_plane && control_plane->is_connected("control-plane")) {
                    edge_sync::protocol::MessageEnvelope hb;
                    hb.set_protocol_version(edge_sync::protocol::kProtocolVersion);
                    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::system_clock::now().time_since_epoch())
                                         .count();
                    hb.mutable_heartbeat()->set_timestamp_ms(
                        static_cast<std::uint64_t>(now));
                    control_plane->send("control-plane", hb);
                }
            }
        });
    }

    logger.log(base::LogLevel::Info, "robot '" + robot_id + "' ready; press Ctrl-C to stop");
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    logger.log(base::LogLevel::Info, "shutting down");
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
    if (heartbeat_thread.joinable()) {
        heartbeat_thread.request_stop();
        heartbeat_thread.join();
    }
    if (control_plane) control_plane->stop();
    peers.stop();
    sqlite.close();
    logger.log(base::LogLevel::Info, "bye");
    return 0;
}
