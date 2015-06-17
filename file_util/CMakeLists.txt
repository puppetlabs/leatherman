find_package(Boost 1.54 REQUIRED COMPONENTS filesystem)

add_leatherman_deps("${Boost_FILESYSTEM_LIBRARY}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

add_leatherman_library("src/directory.cc")
add_leatherman_headers("inc/leatherman")