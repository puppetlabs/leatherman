find_package(Boost 1.54 REQUIRED COMPONENTS regex)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(rapidjson)
leatherman_dependency(valijson)
leatherman_dependency(locale)
leatherman_dependency(util)

add_leatherman_headers(inc/leatherman)

add_leatherman_library(
    src/json_container.cc
    src/schema.cc
    src/validator.cc
    )

add_leatherman_test(
    tests/json_container_test.cc
    tests/schema_test.cc
    tests/validator_test.cc
    )
