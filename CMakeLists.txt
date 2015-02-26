cmake_minimum_required(VERSION 2.8.12)
project(LEATHERMAN)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/internal.cmake")

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

    add_custom_target(cppcheck COMMAND cppcheck --enable=warning,performance --error-exitcode=2 --quiet ${CPPCHECK_DIRS})

    include(FindPythonInterp)
    if (NOT PYTHONINTERP_FOUND)
	message(STATUS "Python not found; 'cpplint' target will not be available")
    else()
	set(CPPLINT_FILTER
            "-build/c++11"            # <thread>, <condvar>, etc...
            "-whitespace/indent"      # We use 4 space indentation
            "-build/include"          # Why?
            "-build/namespaces"       # What's a namespace to do
            "-legal/copyright"        # Not yet
            "-runtime/references"     # Not sure about this religion
            "-readability/streams"    # What?
            "-readability/namespace"  # Ignore nested namespace comment formatting
            "-whitespace/braces"      # Is there a k&r setting?
            "-whitespace/line_length" # Well yeah, but ... not just now
            "-runtime/arrays"         # Sizing an array with a 'const int' doesn't make it variable sized
            "-readability/todo"       # Seriously? todo comments need to identify an owner? pffft
            "-whitespace/empty_loop_body" # Can't handle do { ... } while(expr);
            "-runtime/int"            # Some C types are needed for library interop
            "-runtime/explicit"       # Using implicit conversion from string to regex for regex calls.
            "-build/header_guard"     # Disable header guards (cpplint doesn't yet support enforcing #pragma once)
        )

        file(GLOB_RECURSE ALL_SOURCES */src/*.cc */src/*./h */src/*.hpp */inc/*.hpp */inc/*.h)
	message(STATUS ${ALL_SOURCES})

	set(CPPLINT_PATH "${PROJECT_SOURCE_DIR}/scripts/cpplint.py")

	set(CPPLINT_ARGS "--extensions=cc,hpp,h")
	if (CPPLINT_FILTER)
            string(REPLACE ";" "," CPPLINT_FILTER "${CPPLINT_FILTER}")
            set(CPPLINT_ARGS "${CPPLINT_ARGS};--filter=${CPPLINT_FILTER}")
	endif()
	if (MSVC)
            set(CPPLINT_ARGS "${CPPLINT_ARGS};--output=vs7")
	endif()

	add_custom_target(cpplint
            COMMAND ${PYTHON_EXECUTABLE} ${CPPLINT_PATH} ${CPPLINT_ARGS} ${ALL_SOURCES}
            VERBATIM
	)
    endif()
endif()
