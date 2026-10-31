#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "edge_sync_base/types.hpp"

namespace edge_sync::domain {

struct FileEntry {
    std::string path;
    edge_sync::base::Hash hash;
    std::uint64_t size{0};
    std::uint32_t chunk_count{0};
};

struct Manifest {
    edge_sync::base::ArtifactId artifact_id;
    edge_sync::base::Version version;
    std::vector<FileEntry> files;
};

}  // namespace edge_sync::domain
