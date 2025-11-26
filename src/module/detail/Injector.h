#pragma once

#include "module/detail/get.h"

namespace mod::detail {

template <typename T>
T unwrap(const InjectContext& ctx, Result<T> value) {
    value = std::move(value).or_else([&](auto err) -> Result<T> {
        return error(
            "while loading argument of type {} (index {}): {}",
            rfl::type_name_t<T>().name(),
            ctx.arg_index,
            err);
    });

    if (!value) {
        throw std::move(value.error());
    }

    return std::move(*value);
}

template <typename Module>
struct ValueArgumentInjector {
    explicit ValueArgumentInjector(InjectContext ctx) : ctx(ctx) {}

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Module>)
    operator T() const {
        return unwrap(ctx, get<Module, T>(ctx));
    }

    InjectContext ctx;
};

template <typename Module>
struct RefArgumentInjector {
    explicit RefArgumentInjector(InjectContext ctx) : ctx(ctx) {}

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Module>)
    operator T&() const {
        return *unwrap(ctx, get<Module, T*>(ctx));
    }

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Module>)
    operator const T&() const {
        return *unwrap(ctx, get<Module, const T*>(ctx));
    }

    InjectContext ctx;
};

template <typename T, typename Arg>
using Injector = std::conditional_t<Arg::value, RefArgumentInjector<T>, ValueArgumentInjector<T>>;

}  // namespace mod::detail
