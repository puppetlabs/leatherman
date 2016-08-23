/**
 * @file
 * This header encapsulates our use of chrono. This exists to support the changes
 * that had to be made during PCP-53 where we were forced to switch from using
 * std::thread to boost::thread. This encapsulation means that we can swtich back
 * by changing boost::chrono to std::chrono.
 */

#pragma once

#include <boost/chrono/chrono.hpp>

namespace leatherman { namespace util {

    namespace chrono = boost::chrono;

}}  // namespace leatherman::util
