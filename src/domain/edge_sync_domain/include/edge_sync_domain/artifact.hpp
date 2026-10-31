#pragma once

#include <string>
#include <vector>

#include "edge_sync_base/types.hpp"

namespace edge_sync::domain {

struct Artifact {
    edge_sync::base::ArtifactId artifact_id;
    edge_sync::base::Version version;
    std::vector<std::string> files;
};

}  // namespace edge_sync::domain
