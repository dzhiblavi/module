#include <module/Context.h>
#include <module/config.h>

#include <catch2/catch_all.hpp>

namespace mod {

struct I {
    virtual ~I() = default;
    virtual int foo() = 0;
};

struct CI : I {
    CI(std::vector<int> x, std::vector<int> y) : x_{x}, y_{y} {}
    int foo() override { return x_.back() + y_.front(); }

 private:
    std::vector<int> x_, y_;
};

TEST_CASE("example") {
    rfl::Generic x =
        rfl::Generic::Array{rfl::Generic::VariantType(10), rfl::Generic::VariantType(20)};
    rfl::Generic y =
        rfl::Generic::Array{rfl::Generic::VariantType(100), rfl::Generic::VariantType(200)};

    ModulesConfig config{
        {
            "ci",
            ModuleConfig{
                .cls = "mod::CI",
                .deps = {x, y},
            },
        },
    };

    Context context(config, &Storage::instance());

    auto res = context.loadAllModules();
    INFO(res.error_or(""));
    REQUIRE(res);

    auto mod = context.getModule<I>("ci");
    REQUIRE(mod);

    CHECK(120 == mod.value()->foo());
}

}  // namespace mod

#include <module/register.h>
MODULE_REGISTER(mod::CI);
