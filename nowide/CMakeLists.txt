find_package(Boost 1.54 REQUIRED)

add_leatherman_includes(${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/boost-nowide")
add_leatherman_headers(../vendor/boost-nowide/boost)
if(WIN32)
    add_leatherman_library(../vendor/boost-nowide/libs/nowide/src/iostream.cpp)
endif()
