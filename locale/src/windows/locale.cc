#include <leatherman/locale/locale.hpp>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/locale.hpp>
#pragma GCC diagnostic pop

namespace leatherman { namespace locale {
    std::locale get_locale(std::string const& id)
    {
        // The system default locale is set with id == "", except on Windows boost::locale's
        // generator uses a compatible UTF-8 equivalent. Using boost results in UTF-8 being
        // the default on all platforms.
        return std::locale(boost::locale::generator().generate(id));
    }
}}  // namespace leatherman::locale
