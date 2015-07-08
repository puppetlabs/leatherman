#include <leatherman/util/time.hpp>

namespace leatherman { namespace util {

    void get_local_time(std::time_t* stored_time, std::tm* result){
        localtime_r(stored_time, result);
    }

}}  // namespace leatherman::util
