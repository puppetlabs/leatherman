/**
* @file
* Declares utility functions for setting the locale.
*/
#pragma once

namespace leatherman { namespace locale {

    /**
     * Sets the locale to the specified locale id and imbues it in boost::filesystem.
     * @param id The locale ID, defaults to the system default.
     */
    void set_locale(std::string const& id = "");

}}  // namespace leatherman::locale
