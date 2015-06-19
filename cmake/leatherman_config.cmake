include(leatherman)

# Usage: add_leatherman_deps(${DEP1_LIB} ${DEP2_LIB})
#
# Append to the LEATHERMAN_<LIBRARY>_DEPS variable.
macro(add_leatherman_deps)
    list(APPEND ${deps_var} ${ARGV})
endmacro()

# Usage: add_leatherman_includes(${DIR1} ${DIR2})
#
# Append to the LEATHERMAN_<LIBRARY>_INCLUDE variable
macro(add_leatherman_includes)
    list(APPEND ${include_var} ${ARGV})
    list(REMOVE_DUPLICATES ${include_var})
endmacro()

# Usage: leatherman_dependency("libname")
#
# Automatically handle include directories and library linking for the
# given leatherman library.
#
# Will throw a fatal error if the dependency cannot be found.
macro(leatherman_dependency library)
    string(MAKE_C_IDENTIFIER "${library}" lib)
    string(TOUPPER "${lib}" name)
    set(option "LEATHERMAN_USE_${name}")
    set(dep_lib "LEATHERMAN_${name}_LIB")
    set(dep_deps "LEATHERMAN_${name}_DEPS")
    set(dep_include "LEATHERMAN_${name}_INCLUDE")

    if (NOT "" STREQUAL "${${dep_deps}}")
        debug("Adding ${${dep_deps}} to deps for ${id_upper}")
        list(APPEND ${deps_var} ${${dep_deps}})
    endif()
    if (NOT "" STREQUAL "${${dep_lib}}")
        debug("Adding ${${dep_lib}} to deps for ${id_upper}")
        list(APPEND ${deps_var} ${${dep_lib}})
    endif()
    if (NOT "" STREQUAL "${${dep_include}}")
        debug("Adding ${${dep_include}} to include directories for ${id_upper}")
        list(APPEND ${include_var} ${${dep_include}})
        list(REMOVE_DUPLICATES ${include_var})
    endif()
endmacro()

macro(add_leatherman_library)
    set(${lib_var} "${libname}")
endmacro()

macro(add_leatherman_headers)
endmacro()

macro(add_leatherman_test)
endmacro()

macro(leatherman_component id)
    string(TOUPPER "${id}" id_upper)
    set(include_var "LEATHERMAN_${id_upper}_INCLUDE")
    set(lib_var "LEATHERMAN_${id_upper}_LIB")
    set(deps_var "LEATHERMAN_${id_upper}_DEPS")
    set(include_dir "${LEATHERMAN_PREFIX}/include")
    set(libname "leatherman_${id}")
    set(${include_var} ${include_dir})
    set(${lib_var} "")

    include("${current_directory}/${id}.cmake")

    set(libs_var "LEATHERMAN_${id_upper}_LIBS")
    set(${libs_var} ${${lib_var}} ${${deps_var}})

    if("${ARGV1}" STREQUAL EXCLUDE_FROM_VARS)
        set(exclude_var "LEATHERMAN_EXCLUDE_${id_upper}")
        set(${exclude_var} TRUE)
    endif()
endmacro()
