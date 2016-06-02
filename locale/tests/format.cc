#ifdef LEATHERMAN_I18N
#undef LEATHERMAN_I18N
#endif

#include <catch.hpp>
#include <leatherman/locale/locale.hpp>

using namespace std;
using namespace leatherman::locale;

SCENARIO("a format string", "[locale]") {
    auto literal = "requesting {1} item.";

    GIVEN("basic leatherman::locale::translate") {
        THEN("messages should not be translated") {
            REQUIRE(translate(literal) == literal);
        }

        THEN("messages with context should not be translated") {
            REQUIRE(translate_c("foo", literal) == literal);
        }
    }

    GIVEN("plural leatherman::locale::translate") {
        auto plural = "requesting {1} items.";

        THEN("1 item should be singular") {
            REQUIRE(translate(literal, plural, 1) == literal);
        }

        THEN("0 items should be plural") {
            REQUIRE(translate(literal, plural, 0) == plural);
        }

        THEN("2 items should be plural") {
            REQUIRE(translate(literal, plural, 2) == plural);
        }

        THEN("1 item with context should be singular") {
            REQUIRE(translate_c("foo", literal, plural, 1) == literal);
        }

        THEN("2 items with context should be plural") {
            REQUIRE(translate_c("foo", literal, plural, 2) == plural);
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should perform substitution") {
            REQUIRE(format(literal, 1.25) == "requesting 1.25 item.");
        }
    }
}
