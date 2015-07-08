/**
 * @file
 * Declares utility functions for dealing with time.
 */
#pragma once

#include <string>
#include <ctime>

namespace leatherman { namespace util {

    /**
     * Adds the specified expiry_minutes to the current time and returns
     * the related date time string in UTC format.
     * @return Returns an empty string in case it fails to allocate the buffer.
     */
    std::string get_expiry_datetime(int expiry_minutes = 1);

    /**
     * Gets the current time in ISO8601 format
     * @param modifier_in_secords Offset from the current time in seconds
     * @return Returns the current time plus the modifier
     */
    std::string get_ISO8601_time(unsigned int modifier_in_seconds = 0);

    /** @return Returns the current datetime string in the %Y%m%d_%H%M%S format */
    std::string get_date_time();

    /**
     * Turns a stored time into a local time with correction for timezones.
     *  @param stored_time The time to be converted.
     *  @param result The struct in which to store the local time.
     */
    void get_local_time(std::time_t* stored_time, std::tm* result);

}}  // namespace leatherman::util
