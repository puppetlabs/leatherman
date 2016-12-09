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
            REQUIRE(translate_p("foo", literal) == literal);
        }
    }

    GIVEN("plural leatherman::locale::translate") {
        auto plural = "requesting {1} items.";

        THEN("1 item should be singular") {
            REQUIRE(translate_n(literal, plural, 1) == literal);
        }

        THEN("0 items should be plural") {
            REQUIRE(translate_n(literal, plural, 0) == plural);
        }

        THEN("2 items should be plural") {
            REQUIRE(translate_n(literal, plural, 2) == plural);
        }

        THEN("1 item with context should be singular") {
            REQUIRE(translate_np("foo", literal, plural, 1) == literal);
        }

        THEN("2 items with context should be plural") {
            REQUIRE(translate_np("foo", literal, plural, 2) == plural);
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should perform substitution") {
            REQUIRE(format(literal, 1.25) == "requesting 1.25 item.");
        }

        THEN("messages with context should perform substitution") {
            REQUIRE(format_p("foo", literal, 1.25) == "requesting 1.25 item.");
        }

        /*
         * Apply same tests with *_(...) convenience functions
         */

        THEN("messages should perform substitution") {
            REQUIRE(_(literal, 1.25) == "requesting 1.25 item.");
        }

        THEN("messages with context should perform substitution") {
            REQUIRE(p_("foo", literal, 1.25) == "requesting 1.25 item.");
        }
    }

    GIVEN("plural leatherman::locale::format") {
        auto plural = "requesting {1} items.";

        THEN("1 item should be singular") {
            REQUIRE(format_n(literal, plural, 1, 3.7) == "requesting 3.7 item.");
        }

        THEN("0 item should be plural") {
            REQUIRE(format_n(literal, plural, 0, 3.7) == "requesting 3.7 items.");
        }

        THEN("2 items should be plural") {
            REQUIRE(format_n(literal, plural, 2, 3.7) == "requesting 3.7 items.");
        }

        THEN("1 item with context should be singular") {
            REQUIRE(format_np("foo", literal, plural, 1, 3.7) == "requesting 3.7 item.");
        }

        THEN("2 items with context should be plural") {
            REQUIRE(format_np("foo", literal, plural, 2, 3.7) == "requesting 3.7 items.");
        }
        
        /*
         * Apply same tests with *_(...) convenience functions
         */

        THEN("1 item should be singular") {
            REQUIRE(n_(literal, plural, 1, 3.7) == "requesting 3.7 item.");
        }

        THEN("0 item should be plural") {
            REQUIRE(n_(literal, plural, 0, 3.7) == "requesting 3.7 items.");
        }

        THEN("2 items should be plural") {
            REQUIRE(n_(literal, plural, 2, 3.7) == "requesting 3.7 items.");
        }

        THEN("1 item with context should be singular") {
            REQUIRE(np_("foo", literal, plural, 1, 3.7) == "requesting 3.7 item.");
        }

        THEN("2 items with context should be plural") {
            REQUIRE(np_("foo", literal, plural, 2, 3.7) == "requesting 3.7 items.");
        }
    }
}
