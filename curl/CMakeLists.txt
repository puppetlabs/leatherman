find_package(CURL REQUIRED)

add_leatherman_includes("${CURL_INCLUDE_DIRS}")

leatherman_dependency(logging)
leatherman_dependency(util)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.curl")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library(src/client.cc src/request.cc src/response.cc)
add_leatherman_headers(inc/leatherman)

target_link_libraries(leatherman_curl "${CURL_LIBRARIES}")
