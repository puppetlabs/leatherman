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
        set(LEATHERMAN_LIBRARY_FLAGS "-fPIC -nostdlib -nodefaultlibs")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #set(LEATHERMAN_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
endif()

# Add code coverage
if (COVERALLS)
    set(LEATHERMAN_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if (WIN32)
    # We currently support Windows Server 2003, which requires using deprecated APIs.
    # See http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx for version strings.
    # When Server 2003 support is discontinued, the networking facts implementation can be cleaned up, and
    # we can statically link symbols that are currently being looked up at runtime.
    # add_definitions(-DWINVER=0x0600 -D_WIN32_WINNT=0x0600)

    # The GetUserNameEx function requires the application have a defined security level.
    # We define security sufficient to get the current user's info.
    # Also force use of UNICODE APIs, following the pattern outlined at http://utf8everywhere.org/.
    set(LEATHERMAN_DEFINITIONS -DUNICODE -D_UNICODE -DSECURITY_WIN32)
endif()

list(APPEND LEATHERMAN_DEFINITIONS -DBOOST_LOG_WITHOUT_WCHAR_T)

if (NOT BOOST_STATIC)
    # Boost.Log requires that BOOST_LOG_DYN_LINK is set when using dynamic linking. We set ALL for consistency.
    list(APPEND LEATHERMAN_DEFINITIONS -DBOOST_ALL_DYN_LINK)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
if (WIN32)
    # On Windows, DLL paths aren't hardcoded in the executable. We place all the executables and libraries
    # in the same directory to avoid having to setup the DLL search path in the dev environment.
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
