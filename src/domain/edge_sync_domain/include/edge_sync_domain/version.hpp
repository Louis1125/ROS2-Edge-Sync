#pragma once

#include "edge_sync_base/types.hpp"

namespace edge_sync::domain {

enum class VersionOrder { Older, Equal, Newer, Conflicting };

VersionOrder compare_versions(const edge_sync::base::Version& lhs,
                             const edge_sync::base::Version& rhs);

}  // namespace edge_sync::domain
