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

TEST_CASE("each_line", "[strings]") {

    SECTION("empty string never calls callback") {
        each_line("", [](string &line) {
            FAIL("should not be called");
            return true;
        });
    }

    SECTION("an action is performed on each line") {
        string s = "test1\ntest2\ntest3\n";
        int i = 0;
        each_line(s, [&i](string const &line) {
            i++;
            return line == ("test" + std::to_string(i));
        });
        REQUIRE(i == 3);
    }

    SECTION("a callback that returns false stops at the first line") {
        string s = "test1\ntest2\ntest3\n";
        vector<string> lines;
        each_line(s, [&](string& line) {
            lines.emplace_back(move(line));
            return false;
        });
        REQUIRE(lines.size() == 1u);
        REQUIRE(lines[0] == "test1");
    }

    SECTION("strips '\r' from line endings") {
        string s = "test\r\n";
        each_line(s, [](string& line) {
            REQUIRE(line == "test");
            return true;
        });
    }
}
