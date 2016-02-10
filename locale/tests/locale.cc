#ifndef LEATHERMAN_I18N
#define LEATHERMAN_I18N
#endif

#include <catch.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/locale.hpp>

using namespace std;
using namespace leatherman::locale;

#define _(x) x
static auto literal = _("requesting {1,number}.");
static auto proj = "leatherman_test";

SCENARIO("a default locale") {
    auto loc = get_locale("", proj);
    formatter fmt(proj);

    GIVEN("std::ostream") {
        stringstream ss;
        ss.imbue(loc);
        THEN("messages should not be translated") {
            ss << boost::locale::translate(literal);
            REQUIRE(ss.str() == literal);
        }
    }

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

    GIVEN("std::ostream") {
        stringstream ss;
        ss.imbue(loc);
        THEN("messages should be translated") {
            ss << boost::locale::translate(literal);
            REQUIRE(ss.str() == "demande {1,number}.");
        }
    }

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
            REQUIRE((formatted == "demande 1.25." || formatted == "demande 1,25."));
        }
    }

    clear_domain(proj);
}
