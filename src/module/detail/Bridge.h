#pragma once

#include "module/detail/Interface.h"

namespace mod::detail {

template <typename T>
class ModuleBridge : public T, public Module {
 public:
    using T::T;
};

}  // namespace mod::detail
