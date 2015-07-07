/**
 * @file
 * Declares utility functions for reading data from files.
 */
#pragma once

#include <boost/filesystem/path.hpp>

#include <string>
#include <stdexcept>
#include <functional>

namespace leatherman { namespace file_util {

    /**
     * Reads each line from the given file.
     * @param path The path to the file to read.
     * @param callback The callback function that is passed each line in the file.
     * @return Returns true if the file was opened successfully or false if it was not.
     */
    bool each_line(std::string const& path, std::function<bool(std::string &)> callback);

    /**
     * Reads the entire contents of the given file into a string.
     * @param path The path of the file to read.
     * @return Returns the file contents as a string.
     */
    std::string read(std::string const& path);

    /**
     * Reads the entire contents of the given file into a string.
     * @param path The path of the file to read.
     * @param contents The returned file contents.
     * @return Returns true if the contents were read or false if the file is not readable.
     */
    bool read(std::string const& path, std::string& contents);

    /**
     *@return Returns true if the specified file exists and can
     *      be read by the current process.
     */
    bool file_readable(const std::string &file_path);

    /**
     * Writes content to a temporary file in the specified mode, then
     * renames the file to the desired path. If the file already exists,
     * its previous content will be deleted, so appending is not
     * possible.
     * @param text The content to be written
     * @param file_path The final destination and name of the file
     * @param mode The mode in which to write the file
     *
     * Throws an error in case it fails to open the file to write.
     */
    void atomic_write_to_file(const std::string &text,
                              const std::string &file_path,
                              std::ios_base::openmode mode = std::ios::binary);

    /**
     * Expands a leading tilde to the user's home directory
     * @return Returns the expanded path, or the original string
     *         in case the expansion fails.
     */
    std::string tilde_expand(std::string path);

    /**
     * @return Returns a shell-safe version of the path
     */
    std::string shell_quote(std::string path);

    /**
     * @return Returns the home path for the current platform.
     */
    std::string get_home_path();

}}  // namespace leatherman::file_util
