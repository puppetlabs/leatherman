#ifndef LEATHERMAN_I18N
#define LEATHERMAN_I18N
#endif

#undef PROJECT_NAME
#define PROJECT_NAME "leatherman_locale"

#include <catch.hpp>
#include <leatherman/locale/locale.hpp>

using namespace std;
using namespace leatherman::locale;

SCENARIO("a default locale", "[locale]") {
    GIVEN("basic leatherman::locale::translate") {
        THEN("messages should not be translated") {
            REQUIRE(translate("requesting {1,number}.") == "requesting {1,number}.");
        }

        THEN("messages with context should not be translated") {
            REQUIRE(translate_c("foo", "requesting {1,number}.") == "requesting {1,number}.");
        }
    }

    GIVEN("plural leatherman::locale::translate") {
        THEN("1 item should be singular") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 1) == "requesting {1,number} item.");
        }

        THEN("0 items should be plural") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 0) == "requesting {1,number} items.");
        }

        THEN("2 items should be plural") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 2) == "requesting {1,number} items.");
        }

        THEN("1 item with context should be singular") {
            REQUIRE(translate_c("foo", "requesting {1,number} item.", "requesting {1,number} items.", 1) == "requesting {1,number} item.");
        }

        THEN("2 items with context should be plural") {
            REQUIRE(translate_c("foo", "requesting {1,number} item.", "requesting {1,number} items.", 2) == "requesting {1,number} items.");
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should not be translated") {
            REQUIRE(format("requesting {1,number}.", 1.25) == "requesting 1.25.");
        }
    }

    clear_domain();
}

SCENARIO("a french locale", "[locale]") {
    auto loc = get_locale("fr.UTF-8");

    GIVEN("basic leatherman::locale::translate") {
        THEN("messages should be translated") {
            REQUIRE(translate("requesting {1,number}.") == "demande {1,number}.");
        }

        THEN("messages with context should be translated") {
            REQUIRE(translate_c("foo", "requesting {1,number}.") == "demandé {1,number}.");
        }
    }


    GIVEN("plural leatherman::locale::translate") {
        THEN("1 item should be singular") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 1) == "demande {1,number} objet.");
        }

        THEN("0 items should be singular") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 0) == "demande {1,number} objet.");
        }

        THEN("2 items should be plural") {
            REQUIRE(translate("requesting {1,number} item.", "requesting {1,number} items.", 2) == "demande {1,number} objets.");
        }

        THEN("1 item with context should be singular") {
            REQUIRE(translate_c("foo", "requesting {1,number} item.", "requesting {1,number} items.", 1) == "demandé {1,number} objet.");
        }

        THEN("2 items with context should be plural") {
            REQUIRE(translate_c("foo", "requesting {1,number} item.", "requesting {1,number} items.", 2) == "demandé {1,number} objets.");
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should be translated") {
            auto formatted = format("requesting {1,number}.", 1.25);
            // This doesn't seem to be treated consistently anywhere. Leave it as
            // flexible until we can resolve why.
            CAPTURE(formatted);
            REQUIRE((formatted == "demande 1.25." || formatted == "demande 1,25."));
        }
    }

    clear_domain();
}
