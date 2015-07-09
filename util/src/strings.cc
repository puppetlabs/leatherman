#include <leatherman/util/strings.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <sstream>

using namespace std;

namespace leatherman { namespace util {

    string plural(int num_of_things) {
        return num_of_things == 1 ? "" : "s";
    }

    template<>
    string plural<string>(vector<string> const& things) {
        return plural(things.size());
    }

    string get_UUID() {
        static boost::uuids::random_generator gen;
        boost::uuids::uuid uuid = gen();
        return boost::uuids::to_string(uuid);
    }

    void each_line(string const& s, function<bool(string&)> callback) {
        string line;
        istringstream in(s);
        while (getline(in, line)) {
        // Handle Windows CR in the string.
            if (line.size() && line.back() == '\r') {
                line.pop_back();
            }
            if (!callback(line)) {
                break;
            }
        }
    }

}}  // namespace leatherman::util
