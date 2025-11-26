#pragma once

#include "module/detail/Registerer.h"     // IWYU pragma: keep
#include "module/detail/anon_var_name.h"  // IWYU pragma: keep

#define MODULE_REGISTER(Type)                                        \
    [[maybe_unused]] const auto MODULE_ANON_VAR(module_registerer) = \
        ::mod::detail::Registerer<Type>(#Type)
