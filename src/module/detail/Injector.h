#pragma once

#include "module/Context.h"
#include "module/detail/instance_of_template.h"

#include <rfl/from_generic.hpp>

#include <cstddef>

namespace mod::detail {

struct InjectContext {
    Context* context = nullptr;
    const std::string& name;
    const ModuleConfig& config;
    size_t arg_index = 0;
};

// Value injector (config, string, int etc.)
template <typename Module, typename T>
std::expected<T, std::string> inject(InjectContext ctx) {
    auto&& deps = ctx.config.deps;

    if (ctx.arg_index >= deps.size()) {
        return std::unexpected(
            std::format("dependency index {} is out of range [0..{})", ctx.arg_index, deps.size()));
    }

    auto&& config = ctx.config.deps[ctx.arg_index];

    auto result = rfl::from_generic<T, rfl::DefaultIfMissing>(config);
    if (result) {
        return *result;
    }

    return std::unexpected(result.error()->what());
}

// Dependency injector (shared_ptr)
template <typename Module, osc::utils::InstanceOfTemplate<std::shared_ptr> Dep>
std::expected<Dep, std::string> inject(InjectContext ctx) {
    auto&& deps = ctx.config.deps;

    if (ctx.arg_index >= deps.size()) {
        return std::unexpected(
            std::format("dependency index {} is out of range [0..{})", ctx.arg_index, deps.size()));
    }

    const auto& dep = deps[ctx.arg_index];
    auto name = dep.to_string();
    if (!name) {
        return std::unexpected("string depencency expected");
    }

    return ctx.context->getModule<typename Dep::element_type>(*name);
}

// Dependency injector (weak_ptr)
template <typename Module, osc::utils::InstanceOfTemplate<std::weak_ptr> Dep>
std::expected<Dep, std::string> inject(InjectContext ctx) {
    return inject<Module, std::shared_ptr<typename Dep::element_type>>(ctx);
}

// Dependency injector (raw pointer)
template <typename Module, typename Dep>
requires std::is_pointer_v<Dep>
std::expected<Dep, std::string> inject(InjectContext ctx) {
    return inject<Module, std::shared_ptr<std::remove_pointer_t<Dep>>>(ctx).and_then(
        [](auto m) -> std::expected<Dep, std::string> { return m.get(); });
}

// Dependency injector (vector of shared_ptrs)
template <typename Module, osc::utils::InstanceOfTemplate<std::vector> Deps>
std::expected<Deps, std::string> inject(InjectContext ctx) {
    auto&& deps = ctx.config.deps;
    if (ctx.arg_index >= deps.size()) {
        return Deps{};
    }

    Deps result;
    result.reserve(deps.size() - ctx.arg_index);

    while (ctx.arg_index < deps.size()) {
        auto res = inject<Module, typename Deps::value_type>(ctx);
        if (!res) {
            return std::unexpected(res.error());
        }

        result.push_back(std::move(*res));
        ++ctx.arg_index;
    }

    return result;
}

template <typename T>
T unwrap(const InjectContext& ctx, std::expected<T, std::string> value) {
    value = std::move(value).or_else([&](auto err) -> std::expected<T, std::string> {
        return std::unexpected(
            std::format(
                "while loading argument of type {} (index {}) for module '{}':\n{}",
                rfl::type_name_t<T>().name(),
                ctx.arg_index,
                ctx.name,
                err));
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
        return unwrap(ctx, inject<Module, T>(ctx));
    }

    InjectContext ctx;
};

template <typename Module>
struct RefArgumentInjector {
    explicit RefArgumentInjector(InjectContext ctx) : ctx(ctx) {}

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Module>)
    operator T&() const {
        return *unwrap(ctx, inject<Module, T*>(ctx));
    }

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Module>)
    operator const T&() const {
        return *unwrap(ctx, inject<Module, const T*>(ctx));
    }

    InjectContext ctx;
};

template <typename T, typename Arg>
using Injector = std::conditional_t<Arg::value, RefArgumentInjector<T>, ValueArgumentInjector<T>>;

}  // namespace mod::detail
