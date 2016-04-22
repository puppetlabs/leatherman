#ifndef LEATHERMAN_I18N
#define LEATHERMAN_I18N
#endif

#include <catch.hpp>
#include <leatherman/locale/locale.hpp>

using namespace std;
using namespace leatherman::locale;

#define _(x) x
static auto literal = _("requesting {1,number}.");
static auto proj = "leatherman_test";

SCENARIO("a default locale") {
    auto loc = get_locale("", proj);
    formatter fmt(proj);

    GIVEN("leatherman::locale::translate") {
        THEN("messages should not be translated") {
            REQUIRE(translate(literal, proj) == literal);
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should not be translated") {
            REQUIRE(fmt(literal, 1.25) == "requesting 1.25.");
        }
    }

    clear_domain(proj);
}

SCENARIO("a french locale") {
    auto loc = get_locale("fr.UTF-8", proj);
    formatter fmt(proj);

    GIVEN("leatherman::locale::translate") {
        THEN("messages should be translated") {
            REQUIRE(translate(literal, proj) == "demande {1,number}.");
        }
    }

    GIVEN("leatherman::locale::format") {
        THEN("messages should be translated") {
            auto formatted = fmt(literal, 1.25);
            // This doesn't seem to be treated consistently anywhere. Leave it as
            // flexible until we can resolve why.
            CAPTURE(formatted);
            REQUIRE((formatted == "demande 1.25." || formatted == "demande 1,25."));
        }
    }

    clear_domain(proj);
}
