/**
 * @file
 * Declares utility functions for reading data from files.
 */
#pragma once

#include <boost/filesystem.hpp>

#include <string>
#include <stdexcept>
#include <functional>
#include <wordexp.h>
#include <fstream>

namespace leatherman { namespace file_util {

    /// Generic file error class.
    class file_error : public std::runtime_error {
    public:
        explicit file_error(std::string const& msg) : std::runtime_error(msg) {}
    };

    /**
     * Contains utility functions for reading data from files.
     */
    struct __attribute__((visibility("default"))) file
    {
        /**
         * Reads each line from the given file.
         * @param path The path to the file to read.
         * @param callback The callback function that is passed each line in the file.
         * @return Returns true if the file was opened successfully or false if it was not.
         */
        static bool each_line(std::string const& path, std::function<bool(std::string&)> callback);

        /**
         * Reads the entire contents of the given file into a string.
         * @param path The path of the file to read.
         * @return Returns the file contents as a string.
         */
        static std::string read(std::string const& path);

        /**
         * Reads the entire contents of the given file into a string.
         * @param path The path of the file to read.
         * @param contents The returned file contents.
         * @return Returns true if the contents were read or false if the file is not readable.
         */
        static bool read(std::string const& path, std::string& contents);
    };

    /**
     * Performs a shell expansion of txt.
     * @return Returns an empty string in case of failure.
     */
    std::string shellExpand(std::string txt);

    /**
     *@return Return true if the specified file exists.
     */
    bool fileExists(const std::string& file_path);

    /**
     * Remove a file (regular file, symlink, or empty dir) if exists.
     * Throw a file_error if the removal fails.
     */
    void removeFile(const std::string& file_path);

    /**
     * Write content to file in the specified mode.
     * Throw a file_error in case it fails to open the file to write.
     */
    void streamToFile(const std::string& text,
                      const std::string& file_path,
                      std::ios_base::openmode mode);

    /**
     * Write content to file. If file exists, its previous content will
     * be deleted.
     * Throw a file_error in case it fails to open file to write.
     */
    void writeToFile(const std::string& text,
                     const std::string& file_path);

    /**
     * Read the content of a file.
     * @return Returns file content as a string. Returns the empty string
     *         if the file does not exist.
     */
    std::string readFileAsString(std::string path);

    /**
     * Expand a leading tilde to the user's home directory
     * @return Returns the expanded path, or the original string
     *         in case the expansion fails.
     */
    std::string tildeExpand(std::string path);

    /**
     * Returns a shell-safe version of the path
     */
    std::string shellQuote(std::string path);

    struct FileCopy {
        boost::filesystem::path source;
        std::string relativeName;
    };

    using FileList = std::vector<FileCopy>;

    /**
     * Returns a set of files suitable for copying
     */
    FileList relativeFileList(boost::filesystem::path path);


}}  // namespace leatherman::file_util
