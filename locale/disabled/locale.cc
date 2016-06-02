#include <leatherman/locale/locale.hpp>

namespace leatherman { namespace locale {

    const std::locale get_locale(std::string const& id, std::string const& domain, std::vector<std::string> const& paths)
    {
        // std::locale is not supported on these platforms
        throw std::runtime_error("leatherman::locale::get_locale is not supported on this platform");
    }

    void clear_domain(std::string const& domain)
    {
        throw std::runtime_error("leatherman::locale::clear_domain is not supported on this platform");
    }

    std::string translate(std::string const& msg, std::string const& domain)
    {
        return msg;
    }

    string translate_c(string const& context, string const& msg, string const& domain)
    {
        return msg;
    }

    string translate(string const& single, string const& plural, int n, string const& domain)
    {
        return n == 1 ? single : plural;
    }

    string translate_c(string const& context, string const& single, string const& plural, int n, string const& domain)
    {
        return n == 1 ? single : plural;
    }

}}  // namespace leatherman::locale
