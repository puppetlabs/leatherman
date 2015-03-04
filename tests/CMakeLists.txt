find_package(Threads)

include_directories(${LEATHERMAN_CATCH_INCLUDE} ${LEATHERMAN_INCLUDE_DIRS})
add_executable(leatherman_test main.cc ${LEATHERMAN_TEST_SRCS})

target_link_libraries(leatherman_test ${LEATHERMAN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (COVERALLS)
    target_link_libraries(leatherman_test gcov)
endif()

leatherman_logging_namespace("leatherman.test")
set_target_properties(leatherman_test PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")
add_test(NAME "leatherman\\ tests" COMMAND leatherman_test)