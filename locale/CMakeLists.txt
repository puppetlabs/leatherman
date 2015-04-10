if(WIN32)
    set(LEATHERMAN_LOCALE_SRCS "src/windows/locale.cc")
    find_package(Boost 1.54 REQUIRED COMPONENTS locale)
else()
    set(LEATHERMAN_LOCALE_SRCS "src/posix/locale.cc")
    find_package(Boost 1.54 REQUIRED)
endif()


add_leatherman_includes(${Boost_INCLUDE_DIRS})
if (WIN32)
    add_leatherman_deps("${Boost_LOCALE_LIBRARY}")
endif()

add_leatherman_library(${LEATHERMAN_LOCALE_SRCS})
add_leatherman_headers(inc/leatherman)