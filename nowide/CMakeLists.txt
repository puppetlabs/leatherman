find_package(Boost 1.54 REQUIRED)

add_leatherman_includes(${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/nowide/include")
add_leatherman_headers(../vendor/nowide/include/boost)
if(WIN32)
    add_leatherman_library(../vendor/nowide/src/iostream.cpp)
endif()
