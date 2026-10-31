#include "edge_sync_base/error.hpp"

namespace edge_sync::base {

const char* to_string(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Ok: return "Ok";
        case ErrorCode::InvalidArgument: return "InvalidArgument";
        case ErrorCode::ConnectionFailed: return "ConnectionFailed";
        case ErrorCode::Timeout: return "Timeout";
        case ErrorCode::StorageFailure: return "StorageFailure";
        case ErrorCode::IntegrityFailure: return "IntegrityFailure";
        case ErrorCode::ProtocolMismatch: return "ProtocolMismatch";
        case ErrorCode::VersionConflict: return "VersionConflict";
        case ErrorCode::Internal: return "Internal";
    }
    return "Unknown";
}

}  // namespace edge_sync::base
