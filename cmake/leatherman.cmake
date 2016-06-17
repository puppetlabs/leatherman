# This file contains utilities used by both leatherman and consuming
# projects.

# Save the directory this script is from, to reference other files
# located in the same directory when using cmake in script mode.
set(LEATHERMAN_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

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
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})
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

# Usage: gettext_templates(dir)
#
# Create templates for gettext in `dir` from the source files specified as additional arguments.
# Creates a custom target `translation`.
macro(gettext_templates dir)
    # Don't even try to find gettext on AIX or Solaris, we don't want it.
    if (LEATHERMAN_USE_LOCALES AND LEATHERMAN_GETTEXT)
        find_program(XGETTEXT_EXE xgettext)
    endif()

    if (XGETTEXT_EXE)
        set(TRANSLATION_DIR "${dir}")
        file(MAKE_DIRECTORY ${TRANSLATION_DIR})

        set(ALL_PROJECT_SOURCES ${ARGN})
        set(lang_template ${TRANSLATION_DIR}/${PROJECT_NAME}.pot)
        add_custom_command(OUTPUT ${lang_template}
            COMMAND ${XGETTEXT_EXE}
                --sort-by-file
                --copyright-holder "Puppet \\<docs@puppet.com\\>"
                --package-name=${PROJECT_NAME} --package-version=${PROJECT_VERSION}
                --msgid-bugs-address "docs@puppet.com"
                -d ${PROJECT_NAME} -o ${lang_template}
                --keyword=LOG_DEBUG:1,\\"debug\\"
                --keyword=LOG_INFO:1,\\"info\\"
                --keyword=LOG_WARNING:1,\\"warning\\"
                --keyword=LOG_ERROR:1,\\"error\\"
                --keyword=LOG_FATAL:1,\\"fatal\\"
                --keyword=_
                --keyword=translate:1
                --keyword=translate:1,2
                --keyword=translate_c:1c,2
                --keyword=translate_c:1c,2,3
                --keyword=format
                --add-location=file
                --add-comments=LOCALE
                ${ALL_PROJECT_SOURCES}
            COMMAND ${CMAKE_COMMAND}
                -DPOT_FILE=${lang_template}
                -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
                -P ${LEATHERMAN_CMAKE_DIR}/normalize_pot.cmake
            DEPENDS ${ALL_PROJECT_SOURCES})

        add_custom_target(${PROJECT_NAME}.pot ALL DEPENDS ${lang_template})

        find_program(MSGINIT_EXE msginit)
        find_program(MSGMERGE_EXE msgmerge)
        if (MSGINIT_EXE AND MSGMERGE_EXE)
            foreach(lang ${LEATHERMAN_LOCALES})
                set(lang_file ${TRANSLATION_DIR}/${lang}.po)
                add_custom_command(OUTPUT ${lang_file}
                    COMMAND ${CMAKE_COMMAND}
                        -DPOT_FILE=${lang_template}
                        -DLANG_FILE=${lang_file}
                        -DLANG=${lang}
                        -DMSGMERGE_EXE=${MSGMERGE_EXE}
                        -DMSGINIT_EXE=${MSGINIT_EXE}
                        -P ${LEATHERMAN_CMAKE_DIR}/generate_translations.cmake
                    DEPENDS ${lang_template})
                add_custom_target(${PROJECT_NAME}-${lang}.pot ALL DEPENDS ${lang_file})
            endforeach()
        endif()
    else()
        message(STATUS "Could not find gettext executables, skipping gettext_templates.")
    endif()
endmacro()

# Usage: gettext_compile(dir inst)
#
# Compile gettext .po files into .mo files and configure installing to inst
# Creates a custom target `translations`.
#
# Does nothing if msgfmt (part of gettext) isn't found. Sets GETTEXT_ENABLED
# to ON if we can compile .mo files, otherwise sets to OFF. This variable can
# be used to disable functionality (such as testing) that requires gettext
# translation files.
macro(gettext_compile dir inst)
    # Don't even try to find gettext on AIX or Solaris, we don't want it.
    if (LEATHERMAN_USE_LOCALES AND LEATHERMAN_GETTEXT)
        find_program(MSGFMT_EXE msgfmt)
    endif()

    if (MSGFMT_EXE)
        file(GLOB TRANSLATIONS ${dir}/*.po)
        if (NOT TARGET translations)
            add_custom_target(translations ALL)
        endif()

        # Add LEATHERMAN_LOCALES, as they may not have been generated yet.
        foreach(locale ${LEATHERMAN_LOCALES})
            set(fpath ${dir}/${locale}.po)
            list(FIND TRANSLATIONS ${fpath} FOUND)
            if (${FOUND} EQUAL -1)
                list(APPEND TRANSLATIONS ${fpath})
            endif()
        endforeach()

        foreach(fpath ${TRANSLATIONS})
            get_filename_component(lang ${fpath} NAME_WE)
            set(mo ${CMAKE_BINARY_DIR}/${lang}/LC_MESSAGES/${PROJECT_NAME}.mo)
            add_custom_command(OUTPUT ${mo}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${lang}/LC_MESSAGES
                COMMAND ${MSGFMT_EXE} -c -v -o ${mo} ${fpath} 2>&1
                DEPENDS ${fpath})
            add_custom_target(${lang}-${PROJECT_NAME} DEPENDS ${mo})
            add_dependencies(translations ${lang}-${PROJECT_NAME})
            install(FILES ${mo} DESTINATION "${inst}/${lang}/LC_MESSAGES")
        endforeach()
        set(GETTEXT_ENABLED ON)
    else()
        message(STATUS "Could not find gettext executables, skipping gettext_compile.")
        set(GETTEXT_ENABLED OFF)
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

include(GenerateExportHeader)
# Usage: symbol_exports(TARGET HEADER)
#
# Generate the export header for restricting symbols exported from the library,
# and configure the compiler. Restricting symbols has several advantages, noted
# at https://gcc.gnu.org/wiki/Visibility.
macro(symbol_exports target header)
    generate_export_header(${target} EXPORT_FILE_NAME "${header}")
    # Export on Apple resulted in issues finding symbols from library dependencies
    # that we haven't solved. For now avoid the problem.
    # AIX doesn't support inline headers, and CMake warns if you try to apply the
    # option on static libraries.
    get_target_property(target_type ${target} TYPE)
    if ((NOT APPLE) AND (NOT CMAKE_SYSTEM_NAME MATCHES "AIX") AND
        (${target_type} STREQUAL SHARED_LIBRARY))
        set_target_properties(${target} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)
    endif()
    # If the target name is not a C identifier, generate_export_header will
    # convert it to one. Fix the define to do the same.
    string(MAKE_C_IDENTIFIER ${target} target_c_name)
    string(TOLOWER ${target_c_name} target_name_lower)
    target_compile_definitions(${target} PRIVATE "-D${target_name_lower}_EXPORTS")
endmacro()

# Usage: append_new(VARNAME VAR1 VAR2 ...)
#
# Append ARGN items to VARNAME list if not already present. Accounts for
# optimized/debug flags.
function(append_new varname)
    set(prefix "")
    foreach(DEP ${ARGN})
        if ((${DEP} STREQUAL optimized) OR (${DEP} STREQUAL debug))
            set(prefix ${DEP})
        else()
            list(FIND ${varname} ${DEP} found)
            if (${found} EQUAL -1)
                if (prefix)
                    list(APPEND ${varname} ${prefix})
                endif()
                list(APPEND ${varname} ${DEP})
            endif()
            set(prefix "")
        endif()
    endforeach()
    set(${varname} ${${varname}} PARENT_SCOPE)
endfunction()
