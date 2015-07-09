#include <catch.hpp>
#include <leatherman/util/environment.hpp>
#include <boost/nowide/cenv.hpp>

using namespace std;
using namespace leatherman::util;

SCENARIO("getting an environment variable") {
    string value;
    REQUIRE_FALSE(environment::get("ENVTEST", value));
    REQUIRE(value.empty());
    boost::nowide::setenv("ENVTEST", "FOO", 1);
    REQUIRE(environment::get("ENVTEST", value));
    REQUIRE(value == "FOO");
    boost::nowide::unsetenv("ENVTEST");
    value = "";
    REQUIRE_FALSE(environment::get("ENVTEST", value));
    REQUIRE(value.empty());
}

SCENARIO("setting an environment variable") {
    REQUIRE_FALSE(boost::nowide::getenv(""));
    GIVEN("a non-empty value") {
        REQUIRE(environment::set("ENVTEST", "FOO"));
        THEN("the value is set to the same value") {
            REQUIRE(boost::nowide::getenv("ENVTEST") == string("FOO"));
        }
        boost::nowide::unsetenv("ENVTEST");
    }
    GIVEN("an empty value") {
        REQUIRE(environment::set("ENVTEST", ""));
        THEN("the value is set to empty or not present") {
            string value;
            environment::get("ENVTEST", value);
            REQUIRE(value == "");
        }
        boost::nowide::unsetenv("ENVTEST");
    }
}

SCENARIO("clearing an environment variable") {
    boost::nowide::setenv("ENVTEST", "FOO", 1);
    REQUIRE(environment::clear("ENVTEST"));
    REQUIRE_FALSE(boost::nowide::getenv("ENVTEST"));
}

SCENARIO("enumearing enviornment variables") {
    boost::nowide::setenv("ENVTEST1", "FOO", 1);
    boost::nowide::setenv("ENVTEST2", "BAR", 1);
    boost::nowide::setenv("ENVTEST3", "BAZ", 1);
    WHEN("true is returned from the callback") {
        THEN("all values are returned") {
            string value1;
            string value2;
            string value3;
            environment::each([&](string& name, string& value) {
                if (name == "ENVTEST1") {
                    value1 = move(value);
                } else if (name == "ENVTEST2") {
                    value2 = move(value);
                } else if (name == "ENVTEST3") {
                    value3 = move(value);
                }
                return true;
            });
            REQUIRE(value1 == "FOO");
            REQUIRE(value2 == "BAR");
            REQUIRE(value3 == "BAZ");
        }
    }
    WHEN("false is returned from the callback") {
        THEN("enumeration stops") {
            int count_at_stop = 0;
            int count = 0;
            environment::each([&](string& name, string& value) {
                if (name == "ENVTEST1") {
                    count_at_stop = count;
                    return false;
                }
                ++count;
                return true;
            });
            REQUIRE(count != 0);
            REQUIRE(count == count_at_stop);
        }
    }
    boost::nowide::unsetenv("ENVTEST1");
    boost::nowide::unsetenv("ENVTEST2");
    boost::nowide::unsetenv("ENVTEST3");
}
