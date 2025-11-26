#include <module/Context.h>
#include <module/config.h>

#include <catch2/catch_all.hpp>

namespace mod {

struct I {
    virtual ~I() = default;
    virtual int foo() = 0;
};

struct CI : I {
    CI(std::unordered_map<std::string, int> map) : map_{map} {}
    int foo() override { return map_.size(); }

 private:
    std::unordered_map<std::string, int> map_;
};

TEST_CASE("example") {
    rfl::Object<rfl::Generic> map;
    map.insert(std::make_pair("1", rfl::Generic::VariantType(10)));
    map.insert(std::make_pair("2", rfl::Generic::VariantType(20)));

    ModulesConfig config{
        {
            "ci",
            ModuleConfig{
                .cls = "mod::CI",
                .deps = {map},
            },
        },
    };

    Context context(config, &Storage::instance());

    auto res = context.loadAllModules();
    INFO(res.error_or(""));
    REQUIRE(res);

    auto mod = context.getModule<I>("ci");
    REQUIRE(mod);

    CHECK(2 == mod.value()->foo());
}

}  // namespace mod

#include <module/register.h>
MODULE_REGISTER(mod::CI);
