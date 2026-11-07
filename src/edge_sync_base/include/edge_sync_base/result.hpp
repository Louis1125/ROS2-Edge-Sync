#pragma once

#include <optional>
#include <utility>
#include <variant>

#include "edge_sync_base/error.hpp"

namespace edge_sync::base {

// A value-or-Error result type. Callers check ok() before reading value();
// reading the wrong alternative is a programming error and throws std::bad_variant_access.
template <typename T>
class Result {
   public:
    Result(T value) : storage_(std::move(value)) {}
    Result(Error error) : storage_(std::move(error)) {}

    [[nodiscard]] bool ok() const noexcept {
        return std::holds_alternative<T>(storage_);
    }
    [[nodiscard]] explicit operator bool() const noexcept {
        return ok();
    }

    [[nodiscard]] const T& value() const& {
        return std::get<T>(storage_);
    }
    [[nodiscard]] T& value() & {
        return std::get<T>(storage_);
    }
    [[nodiscard]] T&& value() && {
        return std::get<T>(std::move(storage_));
    }

    [[nodiscard]] const Error& error() const noexcept {
        return std::get<Error>(storage_);
    }

   private:
    std::variant<T, Error> storage_;
};

template <>
class Result<void> {
   public:
    Result() = default;
    Result(Error error) : error_(std::move(error)) {}

    [[nodiscard]] bool ok() const noexcept {
        return !error_.has_value();
    }
    [[nodiscard]] explicit operator bool() const noexcept {
        return ok();
    }
    [[nodiscard]] const Error& error() const& {
        return *error_;
    }

   private:
    std::optional<Error> error_;
};

}  // namespace edge_sync::base
