#ifdef LEATHERMAN_I18N
#undef LEATHERMAN_I18N
#endif

#include <catch.hpp>
#include <leatherman/locale/locale.hpp>

using namespace std;
using namespace leatherman::locale;

SCENARIO("a format string") {
    auto literal = "requesting {1}.";

    GIVEN("leatherman::locale::translate") {
        THEN("messages should not be translated") {
            REQUIRE(translate(literal) == literal);
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should perform substitution") {
            REQUIRE(format(literal, 1.25) == "requesting 1.25.");
        }
    }
}
