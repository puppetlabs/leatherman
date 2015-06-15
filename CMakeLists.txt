cmake_minimum_required(VERSION 2.8.12)
project(LEATHERMAN)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(internal)

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
defoption(LEATHERMAN_INSTALL "Install the leatherman libraries and headers" ${LEATHERMAN_DEFAULT_ENABLE})

set(BUILDING_LEATHERMAN TRUE)

#As with most things, we rely on the containing project to set up the
#common flags
if (LEATHERMAN_TOPLEVEL)
    include(options)
    include(cflags)
endif()

add_definitions(${LEATHERMAN_DEFINITIONS})

add_leatherman_dir(catch EXCLUDE_FROM_VARS)
add_leatherman_dir(nowide)
add_leatherman_dir(locale)
add_leatherman_dir(logging)
add_leatherman_dir(rapidjson)
add_leatherman_dir(json_container)
add_leatherman_dir(util)
if(WIN32)
  add_leatherman_dir(windows)
endif()


list(REVERSE LEATHERMAN_LIBRARIES)
list(REMOVE_DUPLICATES LEATHERMAN_INCLUDE_DIRS)

export_var(LEATHERMAN_INCLUDE_DIRS)
export_var(LEATHERMAN_LIBRARIES)

if(LEATHERMAN_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)

    add_cppcheck_dirs(${LEATHERMAN_CPPCHECK_DIRS})

    file(GLOB_RECURSE ALL_LEATHERMAN_SOURCES */src/*.cc */src/*./h */src/*.hpp */inc/*.hpp */inc/*.h)
    add_cpplint_files(${ALL_LEATHERMAN_SOURCES})

    # If we're toplevel we want to own these targets. If not we assume
    # that containing project will set them up for us.
    if (LEATHERMAN_TOPLEVEL)
	enable_cppcheck()
	enable_cpplint()
    endif()
endif()

# Install the cmake files we need for consumers
if (LEATHERMAN_INSTALL)
    set(CMAKE_FILES
        cmake/cflags.cmake
        cmake/leatherman.cmake
        cmake/options.cmake
        cmake/leatherman_config.cmake
    )
    install(FILES ${CMAKE_FILES} DESTINATION "lib/cmake/leatherman/cmake/")

    configure_file(LeathermanConfig.cmake.in "${PROJECT_BINARY_DIR}/LeathermanConfig.cmake" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/LeathermanConfig.cmake" DESTINATION "lib/cmake/leatherman")
    install(EXPORT LeathermanLibraries DESTINATION "lib/cmake/leatherman")
endif()