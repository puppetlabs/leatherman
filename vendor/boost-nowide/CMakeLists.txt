cmake_minimum_required(VERSION 2.6)

project(boost-nowide)

include_directories(.)
enable_testing()

add_definitions(-DBOOST_NOWIDE_NO_LIB)
option(RUN_WITH_WINE              "Use wine to run tests" OFF)
option(BOOST_NOWIDE_SKIP_TESTS    "Disable running tests" OFF)
option(BOOST_NOWIDE_SKIP_INSTALL  "Disable installation"  OFF)

find_package(Boost 1.48 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if(NOT LIBDIR)
    set(LIBDIR lib CACHE STRING "Library installation directory" FORCE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CXX_FLAGS "-Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CXX_FLAGS "-Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CXX_FLAGS "-Wall")
elseif(MSVC)
    set(CXX_FLAGS "/EHsc /W3")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

set(NOWIDE_TESTS
    test_convert
    test_stdio
    test_fstream
    )

if(NOT BOOST_NOWIDE_SKIP_TESTS)
    foreach(TEST ${NOWIDE_TESTS})
        add_executable(${TEST} test/${TEST}.cpp)
        if(RUN_WITH_WINE)
            add_test(NAME ${TEST} WORKING_DIRECTORY ${CMAKE_BUILD_DIR} COMMAND wine ./${TEST}.exe)
        else()
            add_test(NAME ${TEST} COMMAND ${TEST})
        endif()
    endforeach()

    set(OTHER_TESTS test_env_win test_env_proto)
    add_executable(test_system test/test_system.cpp)

    add_executable(test_env_proto test/test_env.cpp)
    add_executable(test_env_win test/test_env.cpp)
    set_target_properties(test_env_win PROPERTIES COMPILE_DEFINITIONS NOWIDE_TEST_INCLUDE_WINDOWS)
endif()

# The library only has symbols on Windows, so only build on Windows.
if(WIN32 OR RUN_WITH_WINE)
    # Build tests with NOWIDE_WINDOWS enabled
    add_definitions(-DNOWIDE_WINDOWS)

    add_library(nowide-shared SHARED libs/nowide/src/iostream.cpp)
    set_target_properties(nowide-shared PROPERTIES COMPILE_DEFINITIONS BOOST_NOWIDE_DYN_LINK)
    set_target_properties(nowide-shared PROPERTIES
        VERSION 0.0.0
        SOVERSION 0
        CLEAN_DIRECT_OUTPUT 1
        OUTPUT_NAME "nowide"
        )

    add_library(nowide-static STATIC libs/nowide/src/iostream.cpp)
    set_target_properties(nowide-static PROPERTIES
        CLEAN_DIRECT_OUTPUT 1
        OUTPUT_NAME "nowide"
        )

    if(MSVC)
        set_target_properties(nowide-static PROPERTIES PREFIX "lib")
    endif()

    if(NOT BOOST_NOWIDE_SKIP_TESTS)
        add_executable(test_iostream_shared test/test_iostream.cpp)
        set_target_properties(test_iostream_shared PROPERTIES COMPILE_DEFINITIONS BOOST_NOWIDE_DYN_LINK)
        target_link_libraries(test_iostream_shared nowide-shared)

        add_executable(test_iostream_static test/test_iostream.cpp)
        target_link_libraries(test_iostream_static nowide-static)

        list(APPEND OTHER_TESTS test_iostream_shared test_iostream_static)
    endif()
elseif(NOT BOOST_NOWIDE_SKIP_TESTS)
    add_executable(test_iostream test/test_iostream.cpp)
    list(APPEND OTHER_TESTS test_iostream)
endif()


if(NOT BOOST_NOWIDE_SKIP_TESTS)
    if(RUN_WITH_WINE)
        foreach(T ${OTHER_TESTS})
            add_test(NAME ${T} WORKING_DIRECTORY ${CMAKE_BUILD_DIR} COMMAND wine ./${T}.exe)
        endforeach()

        add_test(NAME test_system_n WORKING_DIRECTORY ${CMAKE_BUILD_DIR} COMMAND wine ./test_system.exe "-n")
        add_test(NAME test_system_w WORKING_DIRECTORY ${CMAKE_BUILD_DIR} COMMAND wine ./test_system.exe "-w")
    else()
        foreach(T ${OTHER_TESTS})
            add_test(NAME ${T} COMMAND ${T})
        endforeach()

        add_test(NAME test_system_n COMMAND test_system -n)
        add_test(NAME test_system_w COMMAND test_system -w)
    endif()
endif()

if(NOT BOOST_NOWIDE_SKIP_INSTALL)
    if(WIN32 OR RUN_WITH_WINE)
        install(TARGETS nowide-shared nowide-static
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION ${LIBDIR}
            ARCHIVE DESTINATION ${LIBDIR})
    endif()

    # Install to include/boost/nowide. This essentially patches existing versions of Boost, and may conflict
    # with later versions if nowide is added to Boost.
    install(DIRECTORY boost DESTINATION include)
endif()
