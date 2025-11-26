#pragma once

#include "module/Context.h"
#include "module/Plugin.h"
#include "module/detail/error.h"
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

// Default injector (int, string, struct, ...)
template <typename Module, typename T>
Result<T> getImpl(InjectContext /*ctx*/, const rfl::Generic& param) {
    auto result = rfl::from_generic<T, rfl::DefaultIfMissing>(param);
    if (result) {
        return *result;
    }
    return error(result.error()->what());
}

// Dependency injector (shared_ptr)
template <typename Module, InstanceOfTemplate<std::shared_ptr> Dep>
Result<Dep> getImpl(InjectContext ctx, const rfl::Generic& param) {
    auto name = param.to_string();
    if (!name) {
        return error("string depencency expected");
    }
    return ctx.context->getModule<typename Dep::element_type>(*name);
}

// Dependency injector (weak_ptr)
template <typename Module, InstanceOfTemplate<std::weak_ptr> Dep>
Result<Dep> getImpl(InjectContext ctx, const rfl::Generic& param) {
    return get<Module, std::shared_ptr<typename Dep::element_type>>(ctx, param);
}

// Dependency injector (raw pointer)
template <typename Module, typename Dep>
requires std::is_pointer_v<Dep>
Result<Dep> getImpl(InjectContext ctx, const rfl::Generic& param) {
    return get<Module, std::shared_ptr<std::remove_pointer_t<Dep>>>(ctx, param)
        .and_then([](auto m) { return ok(m.get()); });
}

// Vector
template <typename Module, InstanceOfTemplate<std::vector> Deps>
Result<Deps> getImpl(InjectContext ctx, const rfl::Generic& param) {
    auto maybe_arr = param.to_array();
    if (!maybe_arr) {
        return error("array expected");
    }

    auto arr = *std::move(maybe_arr);
    Deps result;
    result.reserve(arr.size());

    for (auto&& p : arr) {
        auto res = get<Module, typename Deps::value_type>(ctx, p);
        if (!res) {
            return error(res.error());
        }
        result.push_back(*std::move(res));
    }

    return result;
}

// Unordered map
template <typename Module, InstanceOfTemplate<std::unordered_map> Deps>
Result<Deps> getImpl(InjectContext ctx, const rfl::Generic& param) {
    auto maybe_obj = param.to_object();
    if (!maybe_obj) {
        return error("object expected");
    }

    auto obj = *std::move(maybe_obj);
    Deps result;
    result.reserve(obj.size());

    for (auto&& [k, v] : obj) {
        auto key = get<Module, typename Deps::key_type>(ctx, k);
        if (!key) {
            return error(key.error());
        }

        auto value = get<Module, typename Deps::mapped_type>(ctx, v);
        if (!value) {
            return error(value.error());
        }

        result.emplace(*std::move(key), *std::move(value));
    }

    return result;
}

inline Result<rfl::Generic> transform(InjectContext ctx, rfl::Generic param) {
    auto maybe_obj = param.to_object();
    if (!maybe_obj) {
        return param;
    }

    auto obj = *std::move(maybe_obj);
    auto maybe_plugin = obj.get("@plugin");
    if (!maybe_plugin) {
        return param;
    }

    auto plugin_name_obj = *std::move(maybe_plugin);
    auto maybe_plugin_name = plugin_name_obj.to_string();
    if (!maybe_plugin_name) {
        return error("@plugin value must be string");
    }

    auto plugin = ctx.context->getModule<Plugin>(*maybe_plugin_name);
    if (!plugin) {
        return error("while transforming: {}", plugin.error());
    }

    return (*plugin)->transform(std::move(param));
}

template <typename Module, typename T>
Result<T> get(InjectContext ctx, const rfl::Generic& param) {
    Result<rfl::Generic> maybe_param = transform(ctx, param);
    if (!maybe_param) {
        return error(maybe_param.error());
    }

    return getImpl<Module, T>(ctx, *std::move(maybe_param));
}

// Injects nth constructor argument
template <typename Module, typename T>
Result<T> get(InjectContext ctx) {
    auto&& deps = ctx.config.deps;
    if (ctx.arg_index >= deps.size()) {
        return error("dependency index {} is out of range [0..{})", ctx.arg_index, deps.size());
    }

    return get<Module, T>(ctx, ctx.config.deps[ctx.arg_index]);
}

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
