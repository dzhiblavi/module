#pragma once

#include "module/detail/error.h"

#include "module/Context.h"
#include "module/Plugin.h"

#include <rfl/from_generic.hpp>

namespace mod::detail {

inline Result<rfl::Generic> transformOnce(Context* context, rfl::Generic param) {
    auto maybe_obj = param.to_object();
    if (!maybe_obj) {
        return error("object expected (internal)");
    }

    auto obj = *std::move(maybe_obj);
    auto maybe_plugin = obj.get("@plugin");
    if (!maybe_plugin) {
        return error("@plugin expected (internal)");
    }

    auto plugin_name_obj = *std::move(maybe_plugin);
    auto maybe_plugin_name = plugin_name_obj.to_string();
    if (!maybe_plugin_name) {
        return error("@plugin value must be string");
    }

    auto plugin = context->getModule<Plugin>(*maybe_plugin_name);
    if (!plugin) {
        return error("while transforming: {}", plugin.error());
    }

    return (*plugin)->transform(std::move(param));
}

inline Result<rfl::Generic> transformRecursive(Context* context, rfl::Generic param) {
    auto maybe_obj = param.to_object();
    if (!maybe_obj) {
        return param;
    }

    rfl::Generic::Object obj;

    for (auto&& [k, v] : *maybe_obj) {
        auto transformed = transformRecursive(context, std::move(v));
        if (!transformed) {
            return error(transformed.error());
        }

        obj.emplace(std::move(k), *std::move(transformed));
    }

    if (obj.get("@plugin")) {
        return transformOnce(context, std::move(obj));
    }

    return std::move(obj);
}

}  // namespace mod::detail
