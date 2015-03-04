find_package(Boost 1.54 REQUIRED COMPONENTS system filesystem thread)

if(WIN32)
    set(LEATHERMAN_LOCALE_SRCS "src/windows/locale.cc")
else()
    set(LEATHERMAN_LOCALE_SRCS "src/posix/locale.cc")
endif()

include_directories(${Boost_INCLUDE_DIRS})
add_leatherman_deps("${Boost_THREAD_LIBRARY}" "${Boost_SYSTEM_LIBRARY}" "${Boost_FILESYSTEM_LIBRARY}")

add_leatherman_library(${LEATHERMAN_LOCALE_SRCS})

