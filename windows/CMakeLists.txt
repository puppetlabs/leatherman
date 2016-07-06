find_package(Boost 1.54 REQUIRED COMPONENTS filesystem)

add_leatherman_deps(Wbemuuid.lib userenv.lib "${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(util)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.windows")
    leatherman_logging_line_numbers()
endif()

set(WINDOWS_UTIL_SOURCES
  "src/file_util.cc"
  "src/registry.cc"
  "src/process.cc"
  "src/user.cc"
  "src/wmi.cc"
  "src/system_error.cc"
)

add_leatherman_library(${WINDOWS_UTIL_SOURCES})
add_leatherman_headers(inc/leatherman)
add_leatherman_test(tests/file_utils_test.cc)
