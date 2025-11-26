#pragma once

#include "module/detail/Bridge.h"
#include "module/detail/Injector.h"
#include "module/detail/Interface.h"
#include "module/detail/constructor_traits.h"

namespace mod::detail {

template <typename T>
Traits* makeModuleTraits() {
    class TraitsImpl : public Traits {
        using Parameters = typename detail::Constructor<T>::type;
        using Bridge = ModuleBridge<T>;

     public:
        Result<std::shared_ptr<Module>> create(
            Context* context, const ModuleConfig& config, const std::string& name) override {
            try {
                return std::apply(
                    [&, name](auto... args) {
                        return std::make_shared<Bridge>(std::move(args)...);
                    },
                    std::move(makeInjectors(context, config, name)));
            } catch (std::string e) {
                return error(std::move(e));
            }
        }

     private:
        auto makeInjectors(Context* context, const ModuleConfig& config, const std::string& name) {
            InjectContext ctx{
                .context = context,
                .name = name,
                .config = config,
            };

            auto injectors = std::apply(
                [&]<typename... Types>(Types...) {
                    return std::make_tuple(Injector<Bridge, Types>(ctx)...);
                },
                Parameters{});

            std::apply(
                [i = size_t(0)](auto&... args) mutable { ((args.ctx.arg_index = i++), ...); },
                injectors);

            return injectors;
        }
    };

    static TraitsImpl instance;
    return &instance;
}

}  // namespace mod::detail
