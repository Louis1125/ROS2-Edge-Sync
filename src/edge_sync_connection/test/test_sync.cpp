#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <thread>

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/sync_peer_adapter.hpp"
#include "edge_sync_connection/tcp_transport.hpp"
#include "edge_sync_core/sync_coordinator.hpp"
#include "edge_sync_storage/artifact_repository.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

#include "capturing_logger.hpp"

using namespace edge_sync;
using namespace edge_sync::connection;
using namespace edge_sync::core;
using namespace edge_sync::storage;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kRobotB{"127.0.0.1", 19443};
const Endpoint kRobotA{"127.0.0.1", 19444};

std::string tmp_dir(const std::string& tag) {
    auto p = std::filesystem::temp_directory_path() / ("edge_sync_" + tag);
    std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);
    return p.string();
}

void wait_for(const std::function<bool()>& predicate, int attempts = 100) {
    for (int i = 0; i < attempts && !predicate(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}  // namespace

// M1-A vertical slice: a coordinator on robot-a requests a manifest from robot-b
// over a live TCP connection, and robot-b answers from its local repository.
TEST(SyncTest, CoordinatorFetchesRemoteManifest) {
    CapturingLogger log;

    // robot-b owns the artifact.
    const std::string b_root = tmp_dir("sync_b");
    auto b_sqlite = std::make_unique<SqliteRepository>(b_root + "/meta.sqlite");
    ASSERT_TRUE(b_sqlite->open().ok());
    auto b_objects = std::make_unique<ObjectStore>(b_root + "/objects");
    ASSERT_TRUE(b_objects->initialize().ok());
    ArtifactRepository b_repo(*b_sqlite, *b_objects);

    auto hash = b_objects->put(base::ByteBuffer{'m', 'a', 'p'});
    ASSERT_TRUE(hash.ok());
    Manifest m;
    m.artifact_id = "map-warehouse";
    m.version = {1, 4, 0};
    m.files.push_back(FileEntry{"grid.pgm", hash.value(), 3, 1});
    ASSERT_TRUE(b_repo.store(m).ok());

    auto b_conn = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    b_conn->set_local_identity("robot-b");
    ASSERT_TRUE(b_conn->start(kRobotB).ok());
    SyncPeerAdapter b_adapter(*b_conn, b_repo);

    // robot-a starts empty and connects to robot-b.
    const std::string a_root = tmp_dir("sync_a");
    auto a_sqlite = std::make_unique<SqliteRepository>(a_root + "/meta.sqlite");
    ASSERT_TRUE(a_sqlite->open().ok());
    auto a_objects = std::make_unique<ObjectStore>(a_root + "/objects");
    ASSERT_TRUE(a_objects->initialize().ok());
    ArtifactRepository a_repo(*a_sqlite, *a_objects);

    auto a_conn = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    a_conn->set_local_identity("robot-a");
    ASSERT_TRUE(a_conn->start(kRobotA).ok());
    SyncPeerAdapter a_adapter(*a_conn, a_repo);
    ASSERT_TRUE(a_conn->connect_peer(kRobotB).ok());
    wait_for([&] { return a_conn->is_connected("robot-b"); });

    SyncCoordinator coordinator(a_adapter, a_repo);
    auto plan = coordinator.prepare("robot-b", "map-warehouse");

    ASSERT_TRUE(plan.ok()) << plan.error().message;
    EXPECT_EQ(plan.value().artifact_id, "map-warehouse");
    EXPECT_EQ(plan.value().remote_version.major, 1u);
    ASSERT_EQ(plan.value().needed_files.size(), 1u);
    EXPECT_EQ(plan.value().needed_files[0].path, "grid.pgm");
    EXPECT_EQ(plan.value().needed_files[0].hash, hash.value());

    a_conn->stop();
    b_conn->stop();
    a_sqlite->close();
    b_sqlite->close();
}
