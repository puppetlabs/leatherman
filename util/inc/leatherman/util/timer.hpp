/**
 * @file
 * Declares a simple timer class.
 */
#pragma once

#include <chrono>

namespace leatherman { namespace util {

/**
 * A simple stopwatch/timer we can use for user feedback.  We use the
 * std::chrono::steady_clock as we don't want to be affected if the system
 * clock changed around us (think ntp skew/leapseconds).
 */
class Timer {
  public:
    Timer() {
        reset();
    }

    /** @return Returns the time that has passed since last reset in seconds. */
    double elapsed_seconds() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - start_).count();
    }

    /** @return Returns the time that has passed since last reset in milliseconds. */
    int elapsed_milliseconds() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

    /** Resets the clock. */
    void reset() {
        start_ = std::chrono::steady_clock::now();
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> start_;
};

}}  // namespace leatherman::util
