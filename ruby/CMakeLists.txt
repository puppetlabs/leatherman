find_package(Boost 1.54 REQUIRED COMPONENTS regex filesystem system)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(dynamic_library)
leatherman_dependency(util)
leatherman_dependency(execution)

leatherman_dependency(locale)
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
