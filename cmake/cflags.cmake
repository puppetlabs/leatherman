# Set compiler-specific flags
# Each of our project dirs sets CMAKE_CXX_FLAGS based on these. We do
# not set CMAKE_CXX_FLAGS globally because gtest is not warning-clean.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(LEATHERMAN_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-tautological-constant-out-of-range-compare")

    # Clang warns that 'register' is deprecated; 'register' is used throughout boost, so it can't be an error yet.
    # The warning flag is different on different clang versions so we need to extract the clang version.
    # And the Mavericks version of clang report its version in its own special way (at least on 10.9.5) - yay
    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
        string (REGEX REPLACE ".*based on LLVM ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})
    else()
        string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})
    endif()
    MESSAGE( STATUS "CLANG_VERSION_STRING:         " ${CLANG_VERSION_STRING} )

    # Now based on clang version set the appropriate warning flag
    if ("${CLANG_VERSION_STRING}" VERSION_GREATER "3.4")
        set(LEATHERMAN_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS} -Wno-deprecated-register")
    else()
        set(LEATHERMAN_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS} -Wno-deprecated")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # maybe-uninitialized is a relatively new GCC warning that Boost 1.57 violates; disable it for now until it's available in Clang as well
    # it's also sometimes wrong
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")

    # missing-field-initializers is disabled because GCC can't make up their mind how to treat C++11 initializers
    set(LEATHERMAN_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wno-unused-parameter -Wno-unused-local-typedefs -Wno-unknown-pragmas -Wno-missing-field-initializers")
    if (NOT "${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
        set(LEATHERMAN_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS} -Wextra")
    endif()

    # On unix systems we want to be sure to specify -fPIC for libraries
    if (NOT WIN32)
	set(LEATHERMAN_LIBRARY_FLAGS -fPIC)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #set(LEATHERMAN_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
endif()
