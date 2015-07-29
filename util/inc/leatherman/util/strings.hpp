/**
 * @file
 * Declares utility functions for dealing with strings.
 */
#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <functional>
#include <string>
#include <vector>

namespace leatherman { namespace util {

        /**
         * Case-insensitive string comparison.
         */
        struct ciless : std::binary_function<std::string, std::string, bool>
        {
            /**
             * Compares two strings for a "less than" relationship using a case-insensitive comparison.
             * @param s1 The first string to compare.
             * @param s2 The second string to compare.
             * @return Returns true if s1 is less than s2 or false if s1 is equal to or greater than s2.
             */
            bool operator() (const std::string &s1, const std::string &s2) const {
                return boost::lexicographical_compare(s1, s2, boost::is_iless());
            }
        };

        /**
         * @return Returns the "s" string in case of more than one thing,
         *      an empty string otherwise.
         */
        std::string plural(int num_of_things);

        /**
         * @return Returns the "s" string if vector contains more than one item,
         *      an empty string otherwise.
         */
        template<typename T>
        std::string plural(std::vector<T> const& things);

        /** @return Returns universally unique identifier string. */
        std::string get_UUID();

        /**
         * Reads each line from the given string.
         * @param s The string to read.
         * @param callback The callback function that is passed each line in the string.
         */
        void each_line(std::string const& s, std::function<bool(std::string&)> callback);

}}  // namespace leatherman::util
