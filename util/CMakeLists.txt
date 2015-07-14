find_package(Boost 1.54 REQUIRED date_time chrono system)

add_leatherman_deps(${Boost_LIBRARIES})
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

add_leatherman_headers(inc/leatherman)

if(WIN32)
    set(PLATFORM_SRCS "src/windows/time.cc")
else()
    set(PLATFORM_SRCS "src/posix/time.cc")
endif()

add_leatherman_library(src/strings.cc src/time.cc ${PLATFORM_SRCS})

add_leatherman_test(tests/strings-test.cc)