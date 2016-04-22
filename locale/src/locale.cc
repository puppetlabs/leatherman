#include <leatherman/locale/locale.hpp>
#include <map>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <boost/locale.hpp>
#pragma GCC diagnostic pop

namespace leatherman { namespace locale {

    static std::map<std::string, std::locale> g_locales;

    const std::locale get_locale(std::string const& id, std::string const& domain, std::vector<std::string> const& paths)
    {
        auto it = g_locales.find(domain);
        if (it != g_locales.end()) {
            return it->second;
        }

        // The system default locale is set with id == "", except on Windows boost::locale's
        // generator uses a compatible UTF-8 equivalent. Using boost results in UTF-8 being
        // the default on all platforms.
        boost::locale::generator gen;

        if (!domain.empty()) {
#ifdef LEATHERMAN_LOCALE_INSTALL
            // Setup so we can find installed locales.
            gen.add_messages_path(LEATHERMAN_LOCALE_INSTALL);
#endif
            for (auto& path : paths) {
                gen.add_messages_path(path);
            }
            gen.add_messages_domain(domain);
        }

        // Ensure creating and adding a new locale is thread-safe.
        try {
            return g_locales.insert(make_pair(domain, gen(id))).first->second;
        } catch(boost::locale::conv::conversion_error &e) {
            return g_locales.insert(make_pair(domain, std::locale())).first->second;
        }
    }

    void clear_domain(std::string const& domain)
    {
        g_locales.erase(domain);
    }

    std::string translate(std::string const& s, std::string const& domain)
    {
        return boost::locale::translate(s).str(get_locale("", domain));
    }

}}  // namespace leatherman::locale
