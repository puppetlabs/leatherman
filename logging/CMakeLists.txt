find_package(Boost 1.54 REQUIRED COMPONENTS log log_setup thread date_time filesystem system chrono regex)
find_package(Threads)

add_leatherman_deps(${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(locale)

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "SunOS")
    add_leatherman_deps(rt)
endif()

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.logging")
    leatherman_logging_line_numbers()
endif()

if(WIN32)
    set(PLATFORM_SRCS "src/windows/logging.cc")
    set(PLATFORM_TEST_SRCS "tests/windows/logging.cc")
else()
    set(PLATFORM_SRCS "src/posix/logging.cc")
    set(PLATFORM_TEST_SRCS "tests/posix/logging.cc")
endif()

if (LEATHERMAN_USE_LOCALES AND GETTEXT_ENABLED)
    list(APPEND PLATFORM_TEST_SRCS tests/logging_i18n.cc)
endif()

add_leatherman_library(src/logging.cc ${PLATFORM_SRCS})
add_leatherman_test(
    tests/logging.cc
    tests/logging_stream.cc
    tests/logging_stream_lines.cc
    tests/logging_on_message.cc
    ${PLATFORM_TEST_SRCS})
add_leatherman_headers(inc/leatherman)

if (LEATHERMAN_USE_LOCALES AND BUILDING_LEATHERMAN)
    project(leatherman_logging)
    add_subdirectory(locales)
endif()
