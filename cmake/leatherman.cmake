# This file contains utilities used by both leatherman and consuming
# projects.

# Usage: leatherman_logging_namespace("namespace")
#
# Sets the LEATHERMAN_LOGGING_NAMESPACE preprocessor definition to the
# value passed as "namespace".
macro(leatherman_logging_namespace namespace)
    add_definitions("-DLEATHERMAN_LOGGING_NAMESPACE=\"${namespace}\"")
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
