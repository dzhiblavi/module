#pragma once

#include "module/config.h"
#include "module/detail/Interface.h"

#include <memory>

namespace mod {

class Context;

namespace detail {

class Traits {
 public:
    virtual ~Traits() = default;

    virtual Result<std::shared_ptr<Module>> create(
        Context* context, const ModuleConfig& config, const std::string& name) = 0;
};

}  // namespace detail
}  // namespace mod
