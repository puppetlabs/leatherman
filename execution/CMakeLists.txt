find_package(Boost 1.54 REQUIRED COMPONENTS regex filesystem system)

add_leatherman_deps("${Boost_LIBRARIES}")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
    # We use functions provided by this library in the implementation
    # of the create_detached_process execution option on Solaris to
    # execute the child processes in their own contracts
    add_leatherman_deps(contract)
endif()
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(util)
leatherman_dependency(nowide)
leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(file_util)
if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.execution")
    leatherman_logging_line_numbers()
endif()

if(WIN32)
    leatherman_dependency(windows)
endif()

add_leatherman_headers(inc/leatherman)
if(WIN32)
    add_leatherman_library(src/execution.cc src/windows/execution.cc)
else()
    if("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
        # LFS flags are needed to compile the posix/solaris/platform.cc such that it links correctly
        # against the libcontract library. They're not applied universally as they impact the ability
        # to use other OS functions. This usage is safe as long as global variables based on these flags
        # are avoided, according to http://docs.oracle.com/cd/E19455-01/806-0634/6j9vo5alu/index.html
        EXECUTE_PROCESS( COMMAND getconf LFS_CFLAGS OUTPUT_VARIABLE LFS_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
        set(LEATHERMAN_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS} ${LFS_CFLAGS}")
        add_leatherman_library(src/execution.cc src/posix/execution.cc src/posix/solaris/platform.cc)
    else()
        add_leatherman_library(src/execution.cc src/posix/execution.cc src/posix/generic/platform.cc)
    endif()
endif()

if(WIN32)
    set(PLATFORM_TESTS tests/windows/execution.cc)
else()
    set(PLATFORM_TESTS tests/posix/execution.cc)
    if("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
        list(APPEND PLATFORM_TESTS tests/posix/solaris/execution.cc)
    endif()
endif()

add_leatherman_test(tests/log_capture.cc ${PLATFORM_TESTS})

if (BUILDING_LEATHERMAN)
    # Dumb implementation of cat.exe for testing stdin/stdout/stderr handling.
    include_directories(BEFORE ${LEATHERMAN_NOWIDE_INCLUDE})
    add_executable(lth_cat tests/lth_cat.cc)
    target_link_libraries(lth_cat ${LEATHERMAN_NOWIDE_LIBS})
    set_target_properties(lth_cat PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")

    configure_file (
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp.in"
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp"
    )
endif()
