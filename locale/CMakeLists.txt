if(WIN32)
    set(LEATHERMAN_LOCALE_SRCS "src/windows/locale.cc")
    set(PLATFORM_COMPONENTS locale)
else()
    set(LEATHERMAN_LOCALE_SRCS "src/posix/locale.cc")
endif()

find_package(Boost 1.54 REQUIRED COMPONENTS system filesystem thread ${PLATFORM_COMPONENTS})

add_leatherman_includes(${Boost_INCLUDE_DIRS})
add_leatherman_deps("${Boost_THREAD_LIBRARY}" "${Boost_SYSTEM_LIBRARY}" "${Boost_FILESYSTEM_LIBRARY}")
if (WIN32)
    add_leatherman_deps("${Boost_LOCALE_LIBRARY}")
endif()

add_leatherman_library(${LEATHERMAN_LOCALE_SRCS})

