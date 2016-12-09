# This file contains the macros used to add and manage leatherman
# libraries. If you are adding a new library to leatherman, this is
# probably the place to go for documentation. If you're just using
# Leatherman, you should check out the README for information on its
# interface.

include(leatherman) # contains some helpers we use

####
# Macros for use by leatherman libraries
#
# These are the API that libraries use to build themselves as
# "standard" leatherman components"
####

# Usage: add_leatherman_deps(${DEP1_LIB} ${DEP2_LIB})
#
# Append to the LEATHERMAN_<LIBRARY>_DEPS variable.
macro(add_leatherman_deps)
    list(APPEND ${deps_var} ${ARGV})
    export_var(${deps_var})
endmacro()

# Usage: add_leatherman_includes(${DIR1} ${DIR2})
#
# Append to the LEATHERMAN_<LIBRARY>_INCLUDE variable
macro(add_leatherman_includes)
    list(APPEND ${include_var} ${ARGV})
    list(REMOVE_DUPLICATES ${include_var})
    export_var(${include_var})
endmacro()

# Usage: leatherman_dependency("libname")
#
# Automatically handle include directories and library linking for the
# given leatherman library.
#
# Will throw a fatal error if the dependency cannot be found.
macro(leatherman_dependency library)
    string(MAKE_C_IDENTIFIER "${library}" id)
    string(TOUPPER "${id}" name)
    set(option "LEATHERMAN_USE_${name}")
    set(dep_lib "LEATHERMAN_${name}_LIB")
    set(dep_deps "LEATHERMAN_${name}_DEPS")
    set(dep_include "LEATHERMAN_${name}_INCLUDE")

    if(${${option}})
        debug("Found ${library} as ${name}, using it in current context")
        if ((NOT "" STREQUAL "${${dep_deps}}") AND (NOT LEATHERMAN_SHARED))
            debug("Adding ${${dep_deps}} to deps for ${dirname}")
            append_new(${deps_var} ${${dep_deps}})
            export_var(${deps_var})
        endif()
        if (NOT "" STREQUAL "${${dep_lib}}")
            debug("Adding ${${dep_lib}} to deps for ${dirname}")
            list(FIND ${deps_var} ${${dep_lib}} found)
            if (${found} EQUAL -1)
                list(APPEND ${deps_var} ${${dep_lib}})
            endif()
            export_var(${deps_var})
        endif()
        if (NOT "" STREQUAL "${${dep_include}}")
            debug("Adding ${${dep_include}} to include directories for ${dirname}")
            list(APPEND ${include_var} ${${dep_include}})
            list(REMOVE_DUPLICATES ${include_var})
            export_var(${include_var})
        endif()
    else()
        message(FATAL_ERROR "${library} not found as a dependency for ${dirname}")
    endif()
endmacro()

# Usage: add_leatherman_library(${SOURCES} [OPTS] [EXPORTS HEADER])
#
# Creates a static CMake library built from the provided sources. Sets
# LEATHERMAN_<LIBRARY>_LIB to the name of this library.
#
# This macro directly calls add_library, so any add_library options
# can be passed along with the sources.
#
# If the EXPORTS keyword is given, the string following it describes
# the location to put an export header using the symbol_exports
# helper.
#
# This macro cannot be invoked multiple times
macro(add_leatherman_library)
    include_directories(BEFORE ${${include_var}})

    set(LIBRARY_ARGS ${ARGV})
    list(FIND LIBRARY_ARGS EXPORTS EXPORTS_IDX)
    if (NOT ${EXPORTS_IDX} EQUAL -1)
        list(REMOVE_AT LIBRARY_ARGS ${EXPORTS_IDX})
        list(GET LIBRARY_ARGS ${EXPORTS_IDX} EXPORT_HEADER)
        list(REMOVE_AT LIBRARY_ARGS ${EXPORTS_IDX})
    endif()

    if(LEATHERMAN_SHARED)
        add_library(${libname} SHARED ${LIBRARY_ARGS})
        target_link_libraries(${libname} PRIVATE ${${deps_var}})
    else()
        add_library(${libname} STATIC ${LIBRARY_ARGS})
    endif()
    set_target_properties(${libname} PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS} ${LEATHERMAN_LIBRARY_FLAGS}" VERSION ${PROJECT_VERSION})
    if(LEATHERMAN_INSTALL)
        leatherman_install(${libname} EXPORT LeathermanLibraries)
    endif()

    if (EXPORT_HEADER)
        symbol_exports(${libname} ${EXPORT_HEADER})
    endif()
    set(${lib_var} "${libname}" PARENT_SCOPE)
endmacro()

# Usage: add_leatherman_test(${SOURCES} [OPTS])
#
# Adds the listed files to the set which will be built for the
# leatherman unit test executable.
macro(add_leatherman_test)
    foreach(FILE ${ARGV})
        if (IS_ABSOLUTE FILE)
            list(APPEND LEATHERMAN_TEST_SRCS "${FILE}")
        else()
            list(APPEND LEATHERMAN_TEST_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        endif()
    endforeach()
    export_var(LEATHERMAN_TEST_SRCS)
endmacro()


# Usage: add_leatherman_headers(${DIRECTORIES})
#
# Adds the listed directories to the set which will be installed to
# $PREFIX/include
macro(add_leatherman_headers)
    if(LEATHERMAN_INSTALL)
        foreach(DIR ${ARGV})
            install(DIRECTORY "${DIR}" DESTINATION include)
        endforeach()
    endif()
endmacro()

# Usage: add_leatherman_vendored("pkg.zip" "abcdef..." "include" [SOURCE_DIR])
#
# Unpacks a vendored package and installs the headers to include/leatherman/vendor
# Optionally, a variable can be passed as the last argument that's set to the
# unpacked location, in case it's needed for compiling a simple library.
macro(add_leatherman_vendored pkg md5 header_path)
    unpack_vendored(${pkg} ${md5} SOURCE_DIR)

    set(include_dir "${SOURCE_DIR}/${header_path}")
    add_leatherman_includes(${include_dir})

    if (LEATHERMAN_INSTALL)
        install(DIRECTORY "${include_dir}/" DESTINATION "include/leatherman/vendor")
    endif()

    add_custom_target(${dirname} DEPENDS ${pkg})

    if (ARGV4)
        set(${ARGV4} ${SOURCE_DIR})
    endif()
endmacro()

####
# Macros for use in the top-level leatherman CMakeLists.txt
#
# These macros are used to build up the variables which are passed
# into whatever project is including leatherman
####

# Usage: add_leatherman_dir(subdir [EXCLUDE_FROM_VARS])
#
# Creates all of the CMake variables intended to be used by consumers
# of leatherman, including the ENABLE flag.
#
# If the enable flag is set, also sets up the variables used by the
# library API macros and adds the named subdirectory to the CMake
# project.
#
# If EXCLUDE_FROM_VARS is present, this library will not be added to
# the LEATHERMAN_LIBRARIES and LEATHERMAN_INCLUDE variables. The
# LEATHERMAN_<LIBRARY>_<FIELD> variables will still be set.
macro(add_leatherman_dir dir)
    debug("Setting up leatherman library for ${dir}")
    string(MAKE_C_IDENTIFIER "${dir}" id)
    string(TOUPPER "${id}" id_upper)
    set(dirname "${dir}") # Used by other macros to know our human-readable name
    set(option "LEATHERMAN_USE_${id_upper}")
    set(include_dir "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inc")
    set(libname "leatherman_${id}")

    defoption(${option} "Should ${dir} be built and used?" ${LEATHERMAN_DEFAULT_ENABLE})
    if (${${option}})
        set(include_var "LEATHERMAN_${id_upper}_INCLUDE")
        set(lib_var "LEATHERMAN_${id_upper}_LIB")
        set(deps_var "LEATHERMAN_${id_upper}_DEPS")
 
        set(${include_var} ${include_dir})
        set(${lib_var} "") # if library is built, this will be set automatically

        # By adding the subdirectory after setting all variables, but
        # before exporting, we give the library an opportunity to
        # munge them (for example, to add vendor dirs)
        add_subdirectory("${dir}")

        if (LEATHERMAN_INSTALL)
            if ("${ARGV1}" STREQUAL EXCLUDE_FROM_VARS)
                set(COMPONENT_STRING "leatherman_component(${id} EXCLUDE_FROM_VARS)")
            else()
                set(COMPONENT_STRING "leatherman_component(${id})")
            endif()
            install(FILES "${dir}/CMakeLists.txt" DESTINATION "lib${LIB_SUFFIX}/cmake/leatherman" RENAME "${id}.cmake")
            set(LEATHERMAN_COMPONENTS "${LEATHERMAN_COMPONENTS}\n${COMPONENT_STRING}")
        endif()

        # We set this one afterwards because it doesn't need
        # overriding
        set(libs_var "LEATHERMAN_${id_upper}_LIBS")
        set(${libs_var} ${${lib_var}} ${${deps_var}})

        if(NOT "${ARGV1}" STREQUAL EXCLUDE_FROM_VARS)
            debug("Appending values for ${id_upper} to common vars")
            list(APPEND LEATHERMAN_INCLUDE_DIRS ${${include_var}})
            if (NOT "" STREQUAL "${${lib_var}}")
                # Prepend leatherman libraries, as later libs may depend on earlier libs.
                list(INSERT LEATHERMAN_LIBS 0 ${${lib_var}})
            endif()
            append_new(LEATHERMAN_DEPS ${${deps_var}})
        else()
            debug("Excluding values for ${id_upper} from common vars")
        endif()

        export_var(${include_var})
        export_var(${lib_var})
        export_var(${libs_var})
        export_var(${deps_var})

        # Enable cppcheck on this library
        list(APPEND LEATHERMAN_CPPCHECK_DIRS "${CMAKE_SOURCE_DIR}/${dir}")
    endif()
endmacro(add_leatherman_dir)
