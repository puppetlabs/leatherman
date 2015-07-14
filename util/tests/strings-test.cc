#include <catch.hpp>
#include <leatherman/util/strings.hpp>

using namespace std;
using namespace leatherman::util;

TEST_CASE("strings::plural", "[strings]") {
    SECTION("pluralize string based on a number") {
        REQUIRE(plural(1) == "");
        REQUIRE(plural(2) == "s");
        REQUIRE(plural(0) == "s");
    }

    SECTION("pluralize string based on a list") {
        vector<string> things { "thing1" };
        REQUIRE(plural(things) == "");
        things.push_back("thing2");
        REQUIRE(plural(things) == "s");
    }
}

TEST_CASE("strings::get_UUID", "[strings]") {
    SECTION("returns a unique value each time") {
        set<string> ids;
        ids.insert(get_UUID());
        for(int i = 0; i < 100; i++) {
            string new_id = get_UUID();
            REQUIRE(ids.find(new_id) == ids.end());
            ids.insert(new_id);
        }
    }
}
