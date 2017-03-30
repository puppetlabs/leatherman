#include <leatherman/locale/locale.hpp>
#include <leatherman/util/environment.hpp>
#include <map>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <boost/locale.hpp>
#pragma GCC diagnostic pop

namespace leatherman { namespace locale {

    using namespace std;
    static map<string, std::locale> g_locales;

    const std::locale get_locale(string const& id, string const& domain, vector<string> const& paths)
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
            // Setup so we can find installed locales. Expects a default path unless
            // an environment variable is specified.
#ifdef LEATHERMAN_LOCALE_VAR
            string locale_path;
            if (util::environment::get(LEATHERMAN_LOCALE_VAR, locale_path)) {
                gen.add_messages_path(locale_path+'/'+LEATHERMAN_LOCALE_INSTALL);
            }
#else
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

    void clear_domain(string const& domain)
    {
        g_locales.erase(domain);
    }

    string translate(string const& msg, string const& domain)
    {
        try {
            return boost::locale::translate(msg).str(get_locale("", domain));
        } catch (exception const&) {
            return msg;
        }
    }

    string translate_p(string const& context, string const& msg, string const& domain)
    {
        try {
            return boost::locale::translate(context, msg).str(get_locale("", domain));
        } catch (exception const&) {
            return msg;
        }
    }

    string translate_n(string const& single, string const& plural, int n, string const& domain)
    {
        try {
            return boost::locale::translate(single, plural, n).str(get_locale("", domain));
        } catch (exception const&) {
            return n == 1 ? single : plural;
        }
    }

    string translate_np(string const& context, string const& single, string const& plural, int n, string const& domain)
    {
        try {
            return boost::locale::translate(context, single, plural, n).str(get_locale("", domain));
        } catch (exception const&) {
            return n == 1 ? single : plural;
        }
    }

}}  // namespace leatherman::locale
