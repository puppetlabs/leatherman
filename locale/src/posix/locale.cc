#include <leatherman/locale/locale.hpp>

namespace leatherman { namespace locale {
    std::locale get_locale(std::string const& id)
    {
        // Windows uses boost::locale to generate a UTF-8 compatible locale. Other platforms
        // assume UTF-8 should be used, so we can avoid the boost::locale dependency here.
        // GCC 4.8 doesn't yet implement the std::locale(std::string const&) interface, so use c-str.
        return std::locale(id.c_str());
    }
}}  // namespace leatherman::locale
