# This file contains utilities used by both leatherman and consuming
# projects.

# Usage: leatherman_logging_namespace("namespace")
#
# Sets the LEATHERMAN_LOGGING_NAMESPACE preprocessor definition to the
# value passed as "namespace".
macro(leatherman_logging_namespace namespace)
    add_definitions("-DLEATHERMAN_LOGGING_NAMESPACE=\"${namespace}\"")
endmacro()
