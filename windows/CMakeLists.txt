find_package(Boost 1.54 REQUIRED)

add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(logging)
leatherman_dependency(util)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.logging")
    leatherman_logging_line_numbers()
endif()

set(WINDOWS_UTIL_SOURCES
  "src/registry.cc"
  "src/process.cc"
  "src/user.cc"
  "src/wmi.cc"
  "src/system_error.cc"
)

add_leatherman_library(${WINDOWS_UTIL_SOURCES})
add_leatherman_headers(inc/leatherman)

target_link_libraries(leatherman_windows 
	Wbemuuid.lib
	userenv.lib
)
