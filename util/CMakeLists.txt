find_package(Boost 1.54 REQUIRED date_time chrono system)

add_leatherman_deps(${Boost_LIBRARIES})
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)

if(WIN32)
    set(PLATFORM_SRCS "src/windows/time.cc" "src/windows/environment.cc" "src/windows/scoped_handle.cc")
    set(PLATFORM_TESTS "tests/windows/environment.cc")
else()
    set(PLATFORM_SRCS "src/posix/time.cc" "src/posix/environment.cc" "src/posix/scoped_descriptor.cc")
    set(PLATFORM_TESTS "tests/posix/environment.cc")
endif()

add_leatherman_headers(inc/leatherman)

add_leatherman_library(
    src/strings.cc
    src/time.cc
    src/environment.cc
    src/scope_exit.cc
    src/scoped_env.cc
    ${PLATFORM_SRCS}
    )

add_leatherman_test(
    tests/scoped_env.cc
    tests/strings_test.cc
    tests/option_set.cc
    tests/environment.cc
    tests/timer.cc
    ${PLATFORM_TESTS}
    )
