#pragma once

#include "module/detail/error.h"

#include <new>  // IWYU pragma: keep
#include <rfl/Generic.hpp>

namespace mod {

class Plugin {
 public:
    virtual ~Plugin() = default;
    virtual Result<rfl::Generic> transform(rfl::Generic param) = 0;
};

}  // namespace mod
