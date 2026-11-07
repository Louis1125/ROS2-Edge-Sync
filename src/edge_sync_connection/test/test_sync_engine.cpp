#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <thread>

#include "capturing_logger.hpp"
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
using namespace edge_sync::connection;
using namespace edge_sync::core;
using namespace edge_sync::storage;
using edge_sync::test_support::CapturingLogger;

namespace {
const Endpoint kRobotB{"127.0.0.1", 21443};
const Endpoint kRobotA{"127.0.0.1", 21444};

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

base::ByteBuffer make_data(std::size_t n) {
    base::ByteBuffer b(n);
    for (std::size_t i = 0; i < n; ++i) b[i] = static_cast<std::uint8_t>(i * 7);
    return b;
}
}  // namespace

// Full data-plane sync: robot-a runs SyncEngine against robot-b and ends with
// the artifact active and verified.
TEST(SyncEngineTest, SynchronizeEndToEnd) {
    CapturingLogger log;

    const base::ByteBuffer data = make_data(300);
    auto manifest = build_manifest("artifact-x", {1, 0, 0}, data, 32);
    ASSERT_TRUE(manifest.ok());

    // robot-b owns the artifact.
    const std::string b_root = tmp_dir("engine_b");
    auto b_sqlite = std::make_unique<SqliteRepository>(b_root + "/meta.sqlite");
    auto b_objects = std::make_unique<ObjectStore>(b_root + "/objects");
    ArtifactRepository b_repo(*b_sqlite, *b_objects);
    ASSERT_TRUE(b_sqlite->open().ok());
    ASSERT_TRUE(b_objects->initialize().ok());
    for (const auto& c : manifest.value().chunks) {
        base::ByteBuffer chunk(data.begin() + static_cast<std::ptrdiff_t>(c.offset),
                               data.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
        ASSERT_TRUE(b_objects->put(chunk).ok());
    }
    ASSERT_TRUE(b_repo.store(manifest.value()).ok());

    auto b_conn = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    b_conn->set_local_identity("robot-b");
    ASSERT_TRUE(b_conn->start(kRobotB).ok());
    SyncPeerAdapter b_adapter(*b_conn, b_repo, *b_objects);

    // robot-a is empty.
    const std::string a_root = tmp_dir("engine_a");
    auto a_sqlite = std::make_unique<SqliteRepository>(a_root + "/meta.sqlite");
    auto a_objects = std::make_unique<ObjectStore>(a_root + "/objects");
    ArtifactRepository a_repo(*a_sqlite, *a_objects);
    ArtifactStore a_store(a_root + "/artifacts", *a_objects, *a_sqlite);
    ASSERT_TRUE(a_sqlite->open().ok());
    ASSERT_TRUE(a_objects->initialize().ok());
    ASSERT_TRUE(a_store.initialize().ok());

    auto a_conn = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    a_conn->set_local_identity("robot-a");
    ASSERT_TRUE(a_conn->start(kRobotA).ok());
    SyncPeerAdapter a_adapter(*a_conn, a_repo, *a_objects);
    ASSERT_TRUE(a_conn->connect_peer(kRobotB).ok());
    wait_for([&] { return a_conn->is_connected("robot-b"); });

    SyncEngine engine(a_adapter, a_repo, a_adapter, a_store);
    auto outcome = engine.synchronize("robot-b", "artifact-x");

    ASSERT_TRUE(outcome.ok()) << outcome.error().message;
    EXPECT_EQ(outcome.value(), SyncOutcome::Partial);

    auto active = a_store.active_version("artifact-x");
    ASSERT_TRUE(active.ok());
    ASSERT_TRUE(active.value().has_value());
    EXPECT_EQ(active.value()->major, 1u);

    // The active artifact bytes match the original.
    std::ifstream in(a_root + "/artifacts/active/artifact-x", std::ios::binary);
    ASSERT_TRUE(in.good());
    base::ByteBuffer got((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_EQ(got, data);

    a_adapter.stop();
    b_adapter.stop();
    a_conn->stop();
    b_conn->stop();
    a_sqlite->close();
    b_sqlite->close();
}
