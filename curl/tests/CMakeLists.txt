find_package(CURL REQUIRED)

include_directories(BEFORE ${CURL_INCLUDE_DIRS})

if (CURL_STATIC)
    set(CURL_LINK STATIC)
else()
    set(CURL_LINK SHARED)
endif()
add_library(mock_curl ${CURL_LINK} mock_curl.cc)
set_target_properties(mock_curl PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")
if (WIN32)
    symbol_exports(mock_curl "${CMAKE_CURRENT_LIST_DIR}/export.h")
endif()

# Namespacing curl libs so we don't pollute the global namespace
# Needed to properly link libmock_curl in tests
set(LEATHERMAN_INT_CURL_LIBS ${CURL_LIBRARIES})
export_var(LEATHERMAN_INT_CURL_LIBS)

set(LEATHERMAN_TEST_CURL_LIB mock_curl)
export_var(LEATHERMAN_TEST_CURL_LIB)
