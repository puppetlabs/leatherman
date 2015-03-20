/**
* @file
* Declares utility functions for setting the locale.
*/
#pragma once
#include <locale>

namespace leatherman { namespace locale {

    /**
     * Gets a locale object for the specified locale id.
     * @param id The locale ID, defaults to a UTF-8 compatible system default.
     */
    std::locale get_locale(std::string const& id = "");

}}  // namespace leatherman::locale
