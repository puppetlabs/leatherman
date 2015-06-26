find_package(Boost 1.54 REQUIRED)

add_leatherman_includes("${Boost_INCLUDE_DIRS}")

add_leatherman_headers(inc/leatherman)