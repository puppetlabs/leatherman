#include <boost/date_time/posix_time/posix_time.hpp>
#include <leatherman/util/time.hpp>

namespace leatherman { namespace util {

    std::string get_expiry_datetime(int expiry_minutes) {
        struct std::tm expiry_time_info;
        std::string expiry_time_buffer(80, '\0');

        // Get current time and add the specified minutes
        std::time_t expiry_time { time(nullptr) };
        expiry_time += 60 * expiry_minutes;

        // Get local time structure
        get_local_time(&expiry_time, &expiry_time_info);

        // Return the formatted string
        if (strftime(&expiry_time_buffer[0], 80, "%FT%TZ", &expiry_time_info) == 0) {
            // invalid buffer
            return "";
        }

        expiry_time_buffer.resize(strlen(&expiry_time_buffer[0]));
        return expiry_time_buffer;
    }

    std::string get_ISO8601_time(unsigned int modifier_in_seconds) {
        boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time()
                                     + boost::posix_time::seconds(modifier_in_seconds);
        return boost::posix_time::to_iso_extended_string(t) + "Z";
    }

    std::string get_date_time() {
        struct std::tm now_info;
        std::string now_buffer(80, '\0');

        // Get current time
        std::time_t now { time(nullptr) };

        // Get local time structure
        get_local_time(&now, &now_info);

        // Return the formatted string
        strftime(&now_buffer[0], 80, "%Y%m%d_%H%M%S", &now_info);
        now_buffer.resize(strlen(&now_buffer[0]));
        return now_buffer;
    }

}}  // namespace leatherman::util
