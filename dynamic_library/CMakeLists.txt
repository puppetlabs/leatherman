find_package(Boost 1.54 REQUIRED COMPONENTS regex)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(util)
if(WIN32)
    leatherman_dependency(windows)
else()
  if(NOT CMAKE_SYSTEM_NAME MATCHES "FreeBSD|OpenBSD")
      add_leatherman_deps(dl)
  endif()
endif()

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.dynamic_library")
    leatherman_logging_line_numbers()
endif()

add_leatherman_headers(inc/leatherman)
if(WIN32)
    add_leatherman_library(src/windows/dynamic_library.cc src/dynamic_library.cc)
else()
    add_leatherman_library(src/posix/dynamic_library.cc src/dynamic_library.cc)
endif()

add_leatherman_test(tests/dynamic_library_tests.cc)

if (BUILDING_LEATHERMAN)

    #Build dummy dynamic libraries for use in testing
    add_library(libtest SHARED tests/test-lib/hello.cc)
    set_target_properties(libtest PROPERTIES PREFIX "" SUFFIX ".so")
    add_library(libtest1 SHARED tests/test-lib/hello.cc tests/test-lib/goodbye.cc)
    set_target_properties(libtest1 PROPERTIES PREFIX "" SUFFIX ".so")

    configure_file (
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp.in"
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp"
    )
endif()
