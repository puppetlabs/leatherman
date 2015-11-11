find_package(CURL REQUIRED)

if (CURL_STATIC)
    add_definitions(-DCURL_STATICLIB)
    if (WIN32)
        # Linking statically on Windows requires some extra libraries.
        set(CURL_DEPS wldap32.lib ws2_32.lib)
    endif()
endif()

# Namespacing curl libs so we don't pollute the global namespace
# Needed to properly link libmock_curl in tests
set(LEATHERMAN_INT_CURL_LIBS ${CURL_LIBRARIES})
set(LEATHERMAN_TEST_CURL_LIB mock_curl)

export_var(LEATHERMAN_INT_CURL_LIBS)
export_var(LEATHERMAN_TEST_CURL_LIB)

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

add_leatherman_test(tests/client_test.cc tests/request_test.cc tests/response_test.cc)

if (BUILDING_LEATHERMAN)

    # Create a mock curl library; it needs to be separate to allow for dllimport in the client source
    if (CURL_STATIC)
        set(CURL_LINK STATIC)
    else()
        set(CURL_LINK SHARED)
    endif()
    add_library(mock_curl ${CURL_LINK} tests/mock_curl.cc)
    set_target_properties(mock_curl PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")

    if (COVERALLS)
        target_link_libraries(mock_curl gcov)
    endif()
endif()
