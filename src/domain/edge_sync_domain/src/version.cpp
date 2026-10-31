#include "edge_sync_domain/version.hpp"

namespace edge_sync::domain {

VersionOrder compare_versions(const edge_sync::base::Version& lhs,
                              const edge_sync::base::Version& rhs) {
    if (lhs.major != rhs.major) {
        return lhs.major < rhs.major ? VersionOrder::Older : VersionOrder::Newer;
    }
    if (lhs.minor != rhs.minor) {
        return lhs.minor < rhs.minor ? VersionOrder::Older : VersionOrder::Newer;
    }
    if (lhs.patch != rhs.patch) {
        return lhs.patch < rhs.patch ? VersionOrder::Older : VersionOrder::Newer;
    }
    return VersionOrder::Equal;
}

}  // namespace edge_sync::domain
