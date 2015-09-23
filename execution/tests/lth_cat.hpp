#pragma once

#include <string>

namespace lth_cat {
    static const std::string prefix = "Welcome to the Leatherman cat, meow\n";
    static const std::string suffix = "Goodbye\n";
    static const std::string overwhelm = "Overwhelm the read buffer" + std::string(5000, '-');
}  // namespace lth_cat
