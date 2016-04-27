find_package(Boost 1.54 REQUIRED COMPONENTS regex filesystem system)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(util)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.file_util")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library(src/directory.cc src/file.cc)
add_leatherman_headers(inc/leatherman)
add_leatherman_test(tests/file_utils_test.cc tests/directory_utils_test.cc tests/fixtures.cc)
