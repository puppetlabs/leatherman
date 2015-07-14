#include <catch.hpp>
#include <leatherman/util/environment.hpp>
#include <leatherman/util/scoped_env.hpp>

using namespace std;
using namespace leatherman::util;

SCENARIO("scoping an environment variable") {
    string value;
    REQUIRE_FALSE(environment::get("LEATH_ENV_TEST", value));
    REQUIRE(value.empty());

    WHEN("the variable does not exist") {
        AND_WHEN("the variable is scoped") {
            scoped_env foo("LEATH_ENV_TEST", "FOO");
            THEN("the new value is set") {
                REQUIRE(environment::get("LEATH_ENV_TEST", value));
                REQUIRE(value == "FOO");
            }
        }
    }
    WHEN("the variable exists")
    {
        environment::set("LEATH_ENV_TEST", "bar");

        AND_WHEN("the variable is scoped") {
            scoped_env foo("LEATH_ENV_TEST", "FOO");
            THEN("the new value is set") {
                REQUIRE(environment::get("LEATH_ENV_TEST", value));
                REQUIRE(value == "FOO");
            }
        }
        THEN("the variable should be restored") {
            REQUIRE(environment::get("LEATH_ENV_TEST", value));
            REQUIRE(value == "bar");
        }

        environment::clear("LEATH_ENV_TEST");
    }
}
