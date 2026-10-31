#include "edge_sync_storage/object_store.hpp"

#include <filesystem>
#include <fstream>

#include "edge_sync_storage/sha256.hpp"

namespace edge_sync::storage {
namespace fs = std::filesystem;

ObjectStore::ObjectStore(std::string root) : root_(std::move(root)) {}

edge_sync::base::Result<void> ObjectStore::initialize() {
    std::error_code ec;
    fs::create_directories(root_, ec);
    if (ec) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "cannot create object store root: " + ec.message()};
    }
    return {};
}

edge_sync::base::Result<edge_sync::base::Hash> ObjectStore::put(
    const edge_sync::base::ByteBuffer& data) {
    const edge_sync::base::Hash hash = sha256_hex(data);
    const std::string path = path_for(hash);

    std::error_code ec;
    if (fs::exists(path, ec)) return hash;

    fs::create_directories(fs::path(path).parent_path(), ec);
    if (ec) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure, ec.message()};
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "cannot open object file: " + path};
    }
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    if (!out) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "write failed: " + path};
    }
    return hash;
}

edge_sync::base::Result<edge_sync::base::ByteBuffer> ObjectStore::get(
    const edge_sync::base::Hash& hash) {
    std::ifstream in(path_for(hash), std::ios::binary);
    if (!in) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "object not found: " + hash};
    }
    return edge_sync::base::ByteBuffer(std::istreambuf_iterator<char>(in),
                                       std::istreambuf_iterator<char>());
}

edge_sync::base::Result<bool> ObjectStore::exists(const edge_sync::base::Hash& hash) {
    std::error_code ec;
    return fs::exists(path_for(hash), ec);
}

std::string ObjectStore::path_for(const edge_sync::base::Hash& hash) const {
    return root_ + "/" + hash.substr(0, 2) + "/" + hash;
}

}  // namespace edge_sync::storage
