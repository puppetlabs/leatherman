#include <leatherman/util/time.hpp>
#include <windows.h>

namespace leatherman { namespace util {

    void get_local_time(std::time_t* stored_time, std::tm* result){
       localtime_s(result, stored_time);
    }

}}  // namespace leatherman::util
