/**
 * @file
 * Declares tools for deconstructing URIs
 */
#pragma once

#include <string>

namespace leatherman { namespace util {

/**
 * A class that parses a URI into its components.
 * Does not support user_info, and doesn't break fragment out from query.
 */
struct uri
{
    std::string protocol, host, port, path, query;

    uri(std::string const&);

    std::string str() const;
};
}}  // namespace leatherman::util
