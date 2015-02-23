include_directories(${LEATHERMAN_CATCH_INCLUDE})
add_executable(leatherman_test main.cc)
target_link_libraries(leatherman_test ${LEATHERMAN_LIBRARIES} ${LEATHERMAN_TESTLIBS})
add_test(NAME "leatherman\\ tests" COMMAND leatherman_test)