leatherman_dependency(rapidjson)

add_leatherman_library("src/json_container.cc")
add_leatherman_headers("inc/leatherman")
add_leatherman_test("tests/json_container_test.cc")