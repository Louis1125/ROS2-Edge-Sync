#pragma once

#include <string>

namespace edge_sync::base {

enum class ErrorCode {
    Ok = 0,
    InvalidArgument,
    ConnectionFailed,
    Timeout,
    StorageFailure,
    IntegrityFailure,
    ProtocolMismatch,
    VersionConflict,
    Internal,
};

struct Error {
    ErrorCode code{ErrorCode::Ok};
    std::string message;
};

[[nodiscard]] const char* to_string(ErrorCode code) noexcept;

}  // namespace edge_sync::base
