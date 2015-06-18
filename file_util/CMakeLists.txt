find_package(Boost 1.54 REQUIRED COMPONENTS filesystem)

add_leatherman_deps("${Boost_FILESYSTEM_LIBRARY}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(logging)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.logging")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library("src/directory.cc" "src/file.cc")
add_leatherman_headers("inc/leatherman")