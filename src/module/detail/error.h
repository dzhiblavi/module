#pragma once

#include <expected>
#include <format>

namespace mod {

template <typename T>
using Result = std::expected<T, std::string>;

template <typename... Args>
auto error(std::format_string<Args...> fmt, Args&&... args) {
    return std::unexpected(std::format(fmt, std::forward<Args>(args)...));
}

inline auto error(std::string s) {
    return std::unexpected(std::move(s));
}

template <typename T, typename... Args>
auto with(Result<T> res, std::format_string<Args...> fmt, Args&&... args) {
    return std::move(res).or_else([&](auto err) -> Result<T> {
        auto msg = std::format(fmt, std::forward<Args>(args)...);
        return error("{}: {}", std::move(msg), std::move(err));
    });
}

template <typename T>
Result<T> ok(T&& value) {
    return Result<T>(std::forward<T>(value));
}

}  // namespace mod
