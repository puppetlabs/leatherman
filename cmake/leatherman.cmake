# This file contains utilities used by both leatherman and consuming
# projects.

# Usage: leatherman_logging_namespace("namespace")
#
# Sets the LEATHERMAN_LOGGING_NAMESPACE preprocessor definition to the
# value passed as "namespace".
macro(leatherman_logging_namespace namespace)
    add_definitions("-DLEATHERMAN_LOGGING_NAMESPACE=\"${namespace}\"")
endmacro()

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

