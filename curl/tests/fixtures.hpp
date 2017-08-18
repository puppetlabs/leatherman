#pragma once

#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>

/**
 * Class to create a temporary directory with a unique name
 * and destroy it once it is no longer needed.
 *
 * This was taken directly from file_util/tests -- might be
 * worthwhile to include a testutils directory for common test
 * code in leatherman?
 * */
class temp_directory {

public:
    temp_directory();
    ~temp_directory();
    std::string const& get_dir_name() const;

private:
    std::string dir_name;

};

/** Generates a unique string for use as a file path. */
boost::filesystem::path unique_fixture_path();

boost::regex TEMP_FILE_REGEX("\\Atemp_file.*"); 
boost::regex TEMP_DIR_REGEX("\\Afile_util_fixture_.*");
