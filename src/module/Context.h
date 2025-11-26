#pragma once

#include "module/Storage.h"
#include "module/config.h"
#include "module/detail/Interface.h"

#include <rfl/type_name_t.hpp>

namespace mod {

class Context {
 public:
    Context(ModulesConfig config, Storage* storage);

    Result<void> loadAllModules();

    template <typename T>
    Result<std::shared_ptr<T>> getModule(const std::string& name);

 private:
    Result<std::shared_ptr<detail::Module>> getModule(const std::string& name);
    Result<std::shared_ptr<detail::Module>> loadModule(const std::string& name);
    std::optional<ModuleConfig> getConfig(const std::string& name);

    ModulesConfig config_;
    Storage* storage_;
    std::unordered_map<std::string, std::shared_ptr<detail::Module>> modules_;
};

template <typename T>
Result<std::shared_ptr<T>> Context::getModule(const std::string& name) {
    return getModule(name).and_then(
        [&](std::shared_ptr<detail::Module> mod) -> Result<std::shared_ptr<T>> {
            if (auto t = std::dynamic_pointer_cast<T>(std::move(mod))) {
                return ok(std::move(t));
            }

            return error(
                std::format(
                    "type mismatch for module '{}', expected '{}'",
                    name,
                    rfl::type_name_t<T>().str()));
        });
}

}  // namespace mod
