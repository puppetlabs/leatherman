#include <leatherman/util/environment.hpp>
#include <boost/nowide/cenv.hpp>
#include <stdexcept>

using namespace std;

namespace leatherman { namespace util {

    int environment::get_int(string const& name, int default_value)
    {
        auto variable = boost::nowide::getenv(name.c_str());
        if (!variable) {
            return default_value;
        }

        try {
            return stoi(variable);
        }
        catch (invalid_argument&) {
            return default_value;
        }
    }

    bool environment::get(string const& name, string& value)
    {
        auto variable = boost::nowide::getenv(name.c_str());
        if (!variable) {
            return false;
        }

        value = variable;
        return true;
    }

    bool environment::set(string const& name, string const& value)
    {
        return boost::nowide::setenv(name.c_str(), value.c_str(), 1) == 0;
    }

    bool environment::clear(string const& name)
    {
        return boost::nowide::unsetenv(name.c_str()) == 0;
    }

}}  // namespace leatherman::util
