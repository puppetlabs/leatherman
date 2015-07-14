leatherman_dependency(util)
leatherman_dependency(nowide)
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
    add_leatherman_library(src/execution.cc src/posix/execution.cc)
endif()

if(WIN32)
    set(PLATFORM_TESTS tests/windows/execution.cc)
else()
    set(PLATFORM_TESTS tests/posix/execution.cc)
endif()
add_leatherman_test(tests/log_capture.cc ${PLATFORM_TESTS})

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp"
)
