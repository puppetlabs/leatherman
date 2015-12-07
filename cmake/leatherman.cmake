# This file contains utilities used by both leatherman and consuming
# projects.

# Usage: leatherman_logging_namespace("namespace")
#
# Sets the LEATHERMAN_LOGGING_NAMESPACE preprocessor definition to the
# value passed as "namespace".
macro(leatherman_logging_namespace namespace)
    add_definitions("-DLEATHERMAN_LOGGING_NAMESPACE=\"${namespace}\"")
endmacro()

# Usage: leatherman_logging_line_numbers()
#
# Sets the LEATHERMAN_LOGGING_LINE_NUMBERS preprocessor definition.
macro(leatherman_logging_line_numbers)
    add_definitions("-DLEATHERMAN_LOGGING_LINE_NUMBERS")
endmacro()

# Usage: debug("Something cool is happening")
#
# Print message if LEATHERMAN_DEBUG is set. Used to introspect macro
# logic.
macro(debug str)
    if (LEATHERMAN_DEBUG)
	message(STATUS ${str})
    endif()
endmacro(debug)

# Usage: export_var("foobar")
#
# Sets variable "foobar" in the parent scope to the same value as
# "foobar" in the invoking scope. Remember that a macro does not
# create a new scope, but a function does.
macro(export_var varname)
    if (NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
	debug("Exporting ${varname}")
	set(${varname} ${${varname}} PARENT_SCOPE)
    else()
	debug("Skipping export of ${varname} because I'm top-level")
    endif()
    debug("It's value is: ${${varname}}")
endmacro(export_var)

# Usage: defoption(VARNAME "Documentation String" ${DEFAULT_VALUE}")
#
# Define an option that will only be set to DEFAULT_VALUE if it does
# not already exist in this scope. If the variable is available in the
# scope, the option will keep the current value. This works around a
# weird CMake behavior where set(OPTION_VAR TRUE) does not cause
# option() to ignore its default.
macro(defoption name doc default)
    if(DEFINED ${name})
	debug("${name} is already set, using it")
	set(enabled ${${name}})
    else()
	debug("${name} unset, using default")
	set(enabled ${default})
    endif()
    option(${name} ${doc} ${enabled})
endmacro()

# Usage: leatherman_install(TARGETS)
#
# Installs targets using common cross-platform configuration.
# On Windows shared libraries go in bin, import and archive libraries
# go in lib. On Linux shared libraries go in lib. Binaries go in bin.
#
# Also always drop the prefix; give the target its expected name.
# We often have binaries and related dynamic libraries, and this
# simplifies giving them different but related names, such as
# `facter` and `libfacter`.
macro(leatherman_install)
    install(TARGETS ${ARGV}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
    foreach(ARG ${ARGV})
        if (TARGET ${ARG})
            set_target_properties(${ARG} PROPERTIES PREFIX "" IMPORT_PREFIX "")
        endif()
    endforeach()
endmacro()

# Usage: add_cppcheck_dirs(dir1 dir2)
#
# Add the listed directories to the set that cppcheck will be run
# against
macro(add_cppcheck_dirs)
    list(APPEND CPPCHECK_DIRS ${ARGV})
    export_var(CPPCHECK_DIRS)
endmacro()

# Usage: add_cpplint_files(file1 file2)
#
# Add the listed files to the set that cpplint will be run against
macro(add_cpplint_files)
    list(APPEND CPPLINT_FILES ${ARGV})
    export_var(CPPLINT_FILES)
endmacro()

# Usage: enable_cppcheck()
#
# Create the cppcheck custom target with all the directories specified
# in previous calls to `add_cppcheck_dirs`
macro(enable_cppcheck)
    add_custom_target(cppcheck COMMAND cppcheck --enable=warning,performance --error-exitcode=2 --quiet --inline-suppr ${CPPCHECK_DIRS})
endmacro()

# We set this here so that enable_cpplint() can find it
set(LEATHERMAN_CPPLINT_PATH "${CMAKE_CURRENT_LIST_DIR}/../scripts/cpplint.py")

# Usage: enable_cpplint()
#
# Create the cpplint custom target with all the specified in previous
# calls to `add_cpplint_files`
macro(enable_cpplint)
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
            "-runtime/indentation_namespace" # Our namespace indentation is not consistent
            "-readability/inheritance" # virtual/override sometimes used together
            "-whitespace/operators"   # Expects spaces around perfect forwarding (&&)
        )

	set(CPPLINT_ARGS "--extensions=cc,cpp,hpp,h")
	if (CPPLINT_FILTER)
            string(REPLACE ";" "," CPPLINT_FILTER "${CPPLINT_FILTER}")
            set(CPPLINT_ARGS "${CPPLINT_ARGS};--filter=${CPPLINT_FILTER}")
	endif()
	if (MSVC)
            set(CPPLINT_ARGS "${CPPLINT_ARGS};--output=vs7")
	endif()

	add_custom_target(cpplint
            COMMAND ${PYTHON_EXECUTABLE} ${LEATHERMAN_CPPLINT_PATH} ${CPPLINT_ARGS} ${CPPLINT_FILES}
            VERBATIM
	)
    endif()
endmacro()

include(GetGitRevisionDescription)
# Usage: get_commit_string(VARNAME)
#
# Sets VARNAME to the git commit revision string, i.e. (commit SHA1)
function(get_commit_string varname)
    get_git_head_revision(GIT_REFSPEC GIT_SHA1)
    debug("Git SHA1 is ${GIT_SHA1}")
    if ("${GIT_SHA1}" STREQUAL "" OR "${GIT_SHA1}" STREQUAL "GITDIR-NOTFOUND")
        set(${varname} "" PARENT_SCOPE)
    else()
        set(${varname} " (commit ${GIT_SHA1})" PARENT_SCOPE)
    endif()
endfunction()
