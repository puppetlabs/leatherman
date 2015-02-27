find_package(Boost 1.54 REQUIRED COMPONENTS system filesystem)

if(WIN32)
    set(LEATHERMAN_LOCALE_SRCS "src/windows/locale.cc")
else()
    set(LEATHERMAN_LOCALE_SRCS "src/posix/locale.cc")
endif()

include_directories(${Boost_INCLUDE_DIRS})
add_leatherman_deps("${Boost_FILESYSTEM_LIBRARY}" "${Boost_SYSTEM_LIBRARY}")

add_leatherman_library(${LEATHERMAN_LOCALE_SRCS})

