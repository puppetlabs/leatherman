/**
 * @file
 * Declares utility functions for working with files in Windows
 */
#pragma once

#include <string>
#include <stdexcept>

namespace leatherman { namespace windows { namespace file_util {

    struct unknown_folder_exception : public std::runtime_error {
        explicit unknown_folder_exception(const std::string& msg) : std::runtime_error(msg) {}
    };

    /**
     * Finds the ProgramData directory in a Windows-friendly way.
     * @return The ProgramData directory, using the Windows function
     * Throws unknown_folder_exception if SHGetKnownFolderPath fails.
     */
    std::string get_programdata_dir();

}}}  // namespace leatherman::windows::file_util
