find_package(CURL REQUIRED)

if (CURL_STATIC)
    add_definitions(-DCURL_STATICLIB)
    if (WIN32)
        # Linking statically on Windows requires some extra libraries.
        set(CURL_DEPS wldap32.lib ws2_32.lib)
    endif()
endif()

add_leatherman_includes("${CURL_INCLUDE_DIRS}")

leatherman_dependency(logging)
leatherman_dependency(util)
add_leatherman_deps(${CURL_LIBRARIES} ${CURL_DEPS})

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.curl")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library(src/client.cc src/request.cc src/response.cc)
add_leatherman_headers(inc/leatherman)
