#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_connection/sync_peer_adapter.hpp"
#include "edge_sync_connection/tcp_transport.hpp"
#include "edge_sync_core/manifest_builder.hpp"
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
const Endpoint kRobotB{"127.0.0.1", 20443};
const Endpoint kRobotA{"127.0.0.1", 20444};

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

class CollectingReceiver final : public IChunkReceiver {
public:
    edge_sync::base::Result<void> on_chunk(std::uint64_t index, base::ByteBuffer data) override {
        received_[index] = std::move(data);
        return {};
    }
    std::map<std::uint64_t, base::ByteBuffer> received_;
};

// Streams a multi-chunk artifact from robot-b to robot-a with a sliding window.
TEST(TransferTest, StreamsAllChunksWithPipelining) {
    CapturingLogger log;

    base::ByteBuffer data(300);
    for (std::size_t i = 0; i < data.size(); ++i) data[i] = static_cast<std::uint8_t>(i * 7);
    auto manifest = build_manifest("artifact-x", {1, 0, 0}, data, 32);
    ASSERT_TRUE(manifest.ok());
    EXPECT_GT(manifest.value().chunks.size(), 4u);  // larger than the window

    // robot-b owns the artifact.
    const std::string b_root = tmp_dir("transfer_b");
    auto b_sqlite = std::make_unique<SqliteRepository>(b_root + "/meta.sqlite");
    ASSERT_TRUE(b_sqlite->open().ok());
    auto b_objects = std::make_unique<ObjectStore>(b_root + "/objects");
    ASSERT_TRUE(b_objects->initialize().ok());
    ArtifactRepository b_repo(*b_sqlite, *b_objects);
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
    const std::string a_root = tmp_dir("transfer_a");
    auto a_sqlite = std::make_unique<SqliteRepository>(a_root + "/meta.sqlite");
    ASSERT_TRUE(a_sqlite->open().ok());
    auto a_objects = std::make_unique<ObjectStore>(a_root + "/objects");
    ASSERT_TRUE(a_objects->initialize().ok());
    ArtifactRepository a_repo(*a_sqlite, *a_objects);

    auto a_conn = std::make_unique<ConnectionManager>(std::make_unique<TcpTransport>(), log);
    a_conn->set_local_identity("robot-a");
    ASSERT_TRUE(a_conn->start(kRobotA).ok());
    SyncPeerAdapter a_adapter(*a_conn, a_repo, *a_objects);
    ASSERT_TRUE(a_conn->connect_peer(kRobotB).ok());
    wait_for([&] { return a_conn->is_connected("robot-b"); });

    std::vector<std::uint64_t> indices;
    for (const auto& c : manifest.value().chunks) indices.push_back(c.index);

    CollectingReceiver receiver;
    ASSERT_TRUE(a_adapter.pull_chunks("robot-b", "artifact-x", indices, receiver).ok());

    ASSERT_EQ(receiver.received_.size(), manifest.value().chunks.size());
    for (const auto& c : manifest.value().chunks) {
        const auto it = receiver.received_.find(c.index);
        ASSERT_NE(it, receiver.received_.end());
        base::ByteBuffer expected(data.begin() + static_cast<std::ptrdiff_t>(c.offset),
                                  data.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
        EXPECT_EQ(it->second, expected);
    }

    a_adapter.stop();
    b_adapter.stop();
    a_conn->stop();
    b_conn->stop();
    a_sqlite->close();
    b_sqlite->close();
}
