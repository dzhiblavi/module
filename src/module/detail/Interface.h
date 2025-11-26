#pragma once

#include "module/detail/error.h"

#include <memory>

namespace mod::detail {

class Module {
 public:
    virtual ~Module() = default;
};

class Factory : public Module {
 public:
    virtual Result<std::shared_ptr<Module>> modCreate() = 0;
};

}  // namespace mod::detail
