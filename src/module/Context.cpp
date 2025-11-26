#include "module/Context.h"

namespace mod {

Context::Context(Storage* storage) : storage_{storage} {
}

Context::Context(ModulesConfig config, Storage* storage)
    : config_{std::move(config)}
    , storage_{storage} {
}

Result<std::shared_ptr<detail::Module>> Context::getModule(const std::string& name) {
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second;
    }

    return with(loadModule(name), "while loading '{}'", name);
}

Result<std::shared_ptr<detail::Module>> Context::loadModule(const std::string& name) {
    auto maybe_config = getConfig(name);
    if (!maybe_config) {
        return error("not configured");
    }
    auto config = *std::move(maybe_config);

    auto maybe_traits = storage_->get(config.cls);
    if (!maybe_traits) {
        return error("unknown cls '{}'", config.cls);
    }

    return with(maybe_traits.value()->create(this, config, name), "while creating")
        .and_then([&](auto mod) {
            return with(insertModule(name, mod), "while inserting").and_then([&] {
                return ok(std::move(mod));
            });
        });
}

Result<void> Context::insertModule(
    const std::string& name, std::shared_ptr<detail::Module> module) {
    if (modules_.contains(name)) {
        return error("module already exists", name);
    }

    modules_.emplace(name, std::move(module));
    return {};
}

std::optional<ModuleConfig> Context::getConfig(const std::string& name) {
    if (auto it = config_.find(name); it != config_.end()) {
        return it->second;
    }

    return std::nullopt;
}

void Context::setConfig(ModulesConfig config) {
    config_ = std::move(config);
}

Result<void> Context::loadAllModules() {
    for (auto&& [name, config] : config_) {
        if (auto e = getModule(name); !e.has_value()) {
            return error("while loading '{}': {}", name, e.error());
        }
    }

    return {};
}

}  // namespace mod
