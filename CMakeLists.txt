cmake_minimum_required(VERSION 2.8.12)
project(LEATHERMAN)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/leatherman.cmake")

# If we're the top-level project, we want to ensure the build type is
# sane, and flag ourselves as such for later checks
if ("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
    if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "Defaulting to a release build.")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()

    set(LEATHERMAN_TOPLEVEL TRUE)
else()
    set(LEATHERMAN_TOPLEVEL FALSE)
endif()

# If we're the top-level project enable everything by default
defoption(LEATHERMAN_DEFAULT_ENABLE "Should Leatherman libraries all be built by default" ${LEATHERMAN_TOPLEVEL})
defoption(LEATHERMAN_DEBUG "Enable verbose logging messages from leatherman macros" FALSE)
defoption(LEATHERMAN_ENABLE_TESTING "Build the leatherman test binary" ${LEATHERMAN_DEFAULT_ENABLE})

add_leatherman_dir(catch EXCLUDE_FROM_VARS)
add_leatherman_dir(locale)

export_var(LEATHERMAN_INCLUDE_DIRS)
export_var(LEATHERMAN_LIBRARIES)

if(LEATHERMAN_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
