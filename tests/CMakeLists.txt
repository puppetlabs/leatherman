include_directories(BEFORE ${LEATHERMAN_CATCH_INCLUDE} ${LEATHERMAN_INCLUDE_DIRS})
add_executable(leatherman_test main.cc ${LEATHERMAN_TEST_SRCS})

if (LEATHERMAN_SHARED)
    # Include deps first, as they may be static. If they are, linking on Windows can
    # fail due to multiple definitions for the same symbol.
    set(LEATHERMAN_TEST_LIBS ${LEATHERMAN_DEPS} ${LEATHERMAN_LIBS})
else()
    set(LEATHERMAN_TEST_LIBS ${LEATHERMAN_LIBS} ${LEATHERMAN_DEPS})
endif()

if (LEATHERMAN_USE_CURL AND LEATHERMAN_INT_CURL_LIBS AND LEATHERMAN_TEST_CURL_LIB)
    list(REMOVE_ITEM LEATHERMAN_TEST_LIBS ${LEATHERMAN_INT_CURL_LIBS})
    list(APPEND LEATHERMAN_TEST_LIBS ${LEATHERMAN_TEST_CURL_LIB})
endif()

# We link libmock_curl instead of real libcurl in tests
target_link_libraries(leatherman_test ${LEATHERMAN_TEST_LIBS})

leatherman_logging_namespace("leatherman.test")
set_target_properties(leatherman_test PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")
add_test(NAME "leatherman\\ tests" COMMAND leatherman_test)
