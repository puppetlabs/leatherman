#include <leatherman/locale/locale.hpp>

namespace leatherman { namespace locale {
    std::locale get_locale(std::string const& id)
    {
        // The system default locale is set with id == "", except on Windows boost::locale's
        // generator uses a compatible UTF-8 equivalent. Using boost results in UTF-8 being
        // the default on all platforms.
        return std::locale(boost::locale::generator().generate(id));
    }
}}  // namespace leatherman::locale
