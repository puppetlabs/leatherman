find_package(Ruby 1.9 REQUIRED)

leatherman_dependency(dynamic_library)
leatherman_dependency(util)
leatherman_dependency(execution)

leatherman_dependency(logging)
if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.ruby")
    leatherman_logging_line_numbers()
endif()

add_leatherman_headers(inc/leatherman)

if(WIN32)
    set(PLATFORM_SRCS src/windows/api.cc)
else()
    set(PLATFORM_SRCS src/posix/api.cc)
endif()

add_leatherman_library(src/api.cc ${PLATFORM_SRCS})
add_leatherman_test(tests/api-test.cc)