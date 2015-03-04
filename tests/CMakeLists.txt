find_package(Threads)

include_directories(${LEATHERMAN_CATCH_INCLUDE})
add_executable(leatherman_test main.cc)

target_link_libraries(leatherman_test "-Wl,--whole-archive" ${LEATHERMAN_TESTLIBS} "-Wl,--no-whole-archive" ${LEATHERMAN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (COVERALLS)
    target_link_libraries(leatherman_test gcov)
endif()

set_target_properties(leatherman_test PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")
add_test(NAME "leatherman\\ tests" COMMAND leatherman_test)