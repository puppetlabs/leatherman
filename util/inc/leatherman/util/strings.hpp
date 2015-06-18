/**
 * @file
 * Declares utility functions for dealing with strings.
 */
#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <string>

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
}}  // namespace leatherman::util

