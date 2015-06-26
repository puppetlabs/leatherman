#include <leatherman/util/strings.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace leatherman { namespace util {

    std::string plural(int num_of_things) {
        return num_of_things > 1 ? "s" : "";
    }

    template<>
    std::string plural<std::string>(std::vector<std::string> const& things) {
        return plural(things.size());
    }

    std::string get_UUID() {
        static boost::uuids::random_generator gen;
        boost::uuids::uuid uuid = gen();
        return boost::uuids::to_string(uuid);
    }

}}  // namespace leatherman::util
