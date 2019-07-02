cmake_minimum_required(VERSION 3.2.2)
project(leatherman VERSION 1.4.7)

if (WIN32)
    link_libraries("-Wl,--nxcompat -Wl,--dynamicbase")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${PROJECT_BINARY_DIR}/cmake")
# Populate locale install location
configure_file(cmake/leatherman.cmake.in "${PROJECT_BINARY_DIR}/cmake/leatherman.cmake" @ONLY)
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
defoption(LEATHERMAN_SHARED "Create shared libraries instead of static" FALSE)
defoption(LEATHERMAN_USE_ICU "Set when Boost is built with ICU" FALSE)

set(BUILDING_LEATHERMAN TRUE)

#As with most things, we rely on the containing project to set up the
#common flags
if (LEATHERMAN_TOPLEVEL)
    include(options)
    include(cflags)
endif()

if (${LEATHERMAN_SHARED} AND (WIN32 OR ${CURL_STATIC}))
    set(MOCK_CURL FALSE)
else()
    set(MOCK_CURL TRUE)
endif()
defoption(LEATHERMAN_MOCK_CURL "Use mock curl library for testing Leatherman.curl" ${MOCK_CURL})

add_definitions(${LEATHERMAN_DEFINITIONS})
if (LEATHERMAN_LOCALE_VAR AND LEATHERMAN_LOCALE_INSTALL)
    # Add an environment variable to look up install prefix at runtime.
    add_definitions(-DLEATHERMAN_LOCALE_VAR="${LEATHERMAN_LOCALE_VAR}"
                    -DLEATHERMAN_LOCALE_INSTALL="${LEATHERMAN_LOCALE_INSTALL}")
else()
    # Add install location instead.
    add_definitions(-DLEATHERMAN_LOCALE_INSTALL="${CMAKE_INSTALL_PREFIX}/share/locale")
endif()

file(GLOB_RECURSE ALL_LEATHERMAN_SOURCES */src/*.cc */inc/*.hpp)
add_subdirectory(locales)

add_leatherman_dir(catch EXCLUDE_FROM_VARS)
add_leatherman_dir(nowide)
add_leatherman_dir(util)
add_leatherman_dir(locale)
add_leatherman_dir(logging)
add_leatherman_dir(rapidjson)
add_leatherman_dir(json_container)
add_leatherman_dir(file_util)
add_leatherman_dir(curl)
if (WIN32)
  add_leatherman_dir(windows)
endif()
add_leatherman_dir(dynamic_library)
add_leatherman_dir(execution)
add_leatherman_dir(ruby)

# Ensure no LEATHERMAN_LIBS are in LEATHERMAN_DEPS, LEATHERMAN_LIBS should be declared in dependency
# order above, and we don't want them to come after other dependencies.
if (LEATHERMAN_LIBS)
    list(REMOVE_ITEM LEATHERMAN_DEPS ${LEATHERMAN_LIBS})
endif()
if (LEATHERMAN_LIBS OR LEATHERRMAN_DEPS)
    if (LEATHERMAN_SHARED)
        # When using shared libraries, leave out dependencies as they're already handled in the shared libs.
        # Including deps after the shared libraries can lead to "multiple definition" errors.
        list(APPEND LEATHERMAN_LIBRARIES ${LEATHERMAN_LIBS})
    else()
        list(APPEND LEATHERMAN_LIBRARIES ${LEATHERMAN_LIBS} ${LEATHERMAN_DEPS})
    endif()
endif()
if (LEATHERMAN_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES LEATHERMAN_INCLUDE_DIRS)
endif()

export_var(LEATHERMAN_INCLUDE_DIRS)
export_var(LEATHERMAN_LIBRARIES)

if(LEATHERMAN_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)

    add_cppcheck_dirs(${LEATHERMAN_CPPCHECK_DIRS})

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
        cmake/GetGitRevisionDescription.cmake
        cmake/GetGitRevisionDescription.cmake.in
        cmake/pod2man.cmake
        cmake/options.cmake
        cmake/leatherman_config.cmake
        cmake/normalize_pot.cmake
        cmake/generate_translations.cmake
    )
    set(INSTALL_LOC "lib${LIB_SUFFIX}/cmake/leatherman")
    install(FILES
        ${CMAKE_FILES}
        "${PROJECT_BINARY_DIR}/cmake/leatherman.cmake"
        DESTINATION "${INSTALL_LOC}/cmake/")

    configure_file(LeathermanConfig.cmake.in "${PROJECT_BINARY_DIR}/LeathermanConfig.cmake" @ONLY)
    configure_file(LeathermanConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/LeathermanConfigVersion.cmake" @ONLY)
    install(FILES
        "${PROJECT_BINARY_DIR}/LeathermanConfig.cmake"
        "${PROJECT_BINARY_DIR}/LeathermanConfigVersion.cmake"
        DESTINATION ${INSTALL_LOC})
    install(EXPORT LeathermanLibraries DESTINATION ${INSTALL_LOC})

    install(FILES "scripts/cpplint.py" DESTINATION "${INSTALL_LOC}/scripts/")
endif()
