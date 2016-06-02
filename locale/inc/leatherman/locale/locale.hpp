/**
* @file
* Declares utility functions for setting the locale.
*
* Boost.Locale is not available on all platforms. This header is implemented
* so that it can switch between using boost::locale::format and boost::format
* (without localization) by defining LEATHERMAN_I18N. Because these classes
* do not use the same substitution characters, but gettext replacement relies
* on matching a string, specify that both "%N%" (Boost.Format) and "{N}"
* (Boost.Locale) should be considered substitution characters when using
* leatherman::locale::format, and "{N}" should be preferred. When i18n is
* disabled, it will regex replace "{(\d+)}" to "%\1%" for use with Boost.Format.
*/
#pragma once
#include <locale>
#include <vector>
#include <functional>

#ifdef LEATHERMAN_I18N
#include <boost/locale/format.hpp>
#else
#include <boost/format.hpp>
#include <boost/regex.hpp>
// Unset PROJECT_NAME so we only create a single locale.
#undef PROJECT_NAME
#define PROJECT_NAME ""
#endif

namespace leatherman { namespace locale {

    /**
     * Gets a locale object for the specified locale id.
     * @param id The locale ID, defaults to a UTF-8 compatible system default.
     * @param domain The catalog domain to use for i18n via gettext.
     * @param paths Search paths for localization files.
     * @return The locale. If a locale for the specified domain already exists, it returns
     * the same locale until clear_domain is called for that domain.
     * Throws boost::locale::conv::conversion_error if the system locale is invalid or
     * the catalog for the specified language can't be used with the system locale encoding.
     *
     * Unsafe to use with GCC on AIX or Solaris, as std::locale is busted.
     */
    const std::locale get_locale(std::string const& id = "",
                                 std::string const& domain = PROJECT_NAME,
                                 std::vector<std::string> const& paths = {PROJECT_DIR});

    /**
     * Clears the locale for a specific domain.
     * WARNING: This may invalidate existing references, so only use for testing.
     * @param domain The catalog domain to clear.
     */
    void clear_domain(std::string const& domain = PROJECT_NAME);

    /**
     * Translate text using the locale initialized by this library.
     * If localization encounters an error, the original message will be returned.
     * @param msg The string to translate.
     * @param domain The catalog domain to use for i18n via gettext.
     * @return The translated string.
     */
    std::string translate(std::string const& msg, std::string const& domain = PROJECT_NAME);

    /**
     * Translate text in a given context using the locale initialized by this library.
     * Context can be used to disambiguate the same word used multiple different ways.
     * If localization encounters an error, the original message will be returned.
     * @param context The context string.
     * @param msg The string to translate.
     * @param domain The catalog domain to use for i18n via gettext.
     * @return The translated string.
     */
    std::string translate_c(std::string const& context, std::string const& msg, std::string const& domain = PROJECT_NAME);

    /**
     * Translate plural text using the locale initialized by this library.
     * If localization encounters an error, the `single` message will be returned for n == 1,
     * and the `plural` message will be returned for all other values of n.
     * @param single The singuar form to translate.
     * @param plural The plural form to translate.
     * @param n Number of items, used to choose singular or plural.
     * @param domain The catalog domain to use for i18n via gettext.
     * @return The translated string.
     */
    std::string translate(std::string const& single, std::string const& plural, int n, std::string const& domain = PROJECT_NAME);

    /**
     * Translate plural text in a given context using the locale initialized by this library.
     * Context can be used to disambiguate the same word used multiple different ways.
     * If localization encounters an error, the `single` message will be returned for n == 1,
     * and the `plural` message will be returned for all other values of n.
     * @param context The context string.
     * @param single The singuar form to translate.
     * @param plural The plural form to translate.
     * @param n Number of items, used to choose singular or plural.
     * @param domain The catalog domain to use for i18n via gettext.
     * @return The translated string.
     */
    std::string translate_c(std::string const& context, std::string const& single, std::string const& plural, int n, std::string const& domain = PROJECT_NAME);

#ifdef LEATHERMAN_I18N
    /**
     * A formatter class for translating and formatting text according to a domain.
     */
    class formatter
    {
        std::string domain_;

    public:
        /**
         * Construct a new formatter.
         * @param domain The domain to use for localization.
         */
        formatter(std::string domain = PROJECT_NAME) : domain_(std::move(domain)) {}

        /**
         * Translate and format.
         * @param s The format string.
         * @param args Format arguments.
         * @return The string generated by translating the format string, then applying the arguments.
         */
        template <typename... TArgs>
        std::string operator()(std::string const& s, TArgs... args)
        {
            // Create and apply formatter here, as we want to guarantee the lifetime of the arguments.
            // boost::locale::format doesn't make copies, and a common gotcha is using temporary arguments
            // to build up the formatter.
            boost::locale::format form(translate(s, domain_));
            // Technique for the one-liner explained at http://florianjw.de/en/variadic_templates.html.
            (void) std::initializer_list<int>{ ((void)(form % args), 0)... };
            return form.str(get_locale("", domain_));
        }
    };
#endif

    // Use the default domain, i.e. PROJECT_NAME
    /**
     * Translates and formats text using the locale initialized by this library.
     * Replaces the use of boost::format with a variadic function call.
     * Specialized to the PROJECT_NAME domain.
     * @param s The format string.
     * @param args Format arguments.
     * @return The string generated by translating the format string, then applying the arguments.
     */
    template <typename... TArgs>
    static std::string format(std::string const& s, TArgs... args)
    {
#ifdef LEATHERMAN_I18N
        return formatter{}(s, std::forward<TArgs>(args)...);
#else
        boost::regex expr{"\\{(\\d+)\\}"};
        boost::format form{boost::regex_replace(s, expr, "%\\1%")};
        (void) std::initializer_list<int>{ ((void)(form % args), 0)... };
        return form.str();
#endif
    }

}}  // namespace leatherman::locale
