find_package(Boost 1.54 REQUIRED COMPONENTS regex)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

if (BUILDING_LEATHERMAN AND LEATHERMAN_MOCK_CURL)
    # Create a mock curl library; it needs to be separate to allow for dllimport in the client source
    # Do it first to avoid symbol_exports defined later.
    add_subdirectory(tests)
    export_var(LEATHERMAN_INT_CURL_LIBS)
    export_var(LEATHERMAN_TEST_CURL_LIB)
endif()

find_package(CURL REQUIRED)

if (CURL_STATIC)
    add_definitions(-DCURL_STATICLIB)
    if (WIN32)
        # Linking statically on Windows requires some extra libraries.
        set(CURL_DEPS wldap32.lib ws2_32.lib)
    endif()
endif()

add_leatherman_includes("${CURL_INCLUDE_DIRS}")

leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(util)
add_leatherman_deps(${CURL_LIBRARIES} ${CURL_DEPS})

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.curl")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library(src/client.cc src/request.cc src/response.cc EXPORTS "${CMAKE_CURRENT_LIST_DIR}/inc/leatherman/curl/export.h")
add_leatherman_headers(inc/leatherman)

if (BUILDING_LEATHERMAN AND LEATHERMAN_MOCK_CURL)
    add_leatherman_test(tests/client_test.cc tests/request_test.cc tests/response_test.cc)
endif()
