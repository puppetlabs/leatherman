if (LEATHERMAN_ENABLE_LOCALE)
    find_package(Boost 1.54 REQUIRED COMPONENTS locale system)
else()
    find_package(Boost 1.54 REQUIRED regex)
endif()

add_leatherman_includes(${Boost_INCLUDE_DIRS})
add_leatherman_deps(${Boost_LIBRARIES})

add_leatherman_headers(inc/leatherman)

if (LEATHERMAN_ENABLE_LOCALE)
    add_leatherman_library(src/locale.cc)
    add_leatherman_test(tests/locale.cc)
else()
    add_leatherman_library(disabled/locale.cc)
endif()

add_leatherman_test(tests/format.cc)

if (BUILDING_LEATHERMAN)
    project(leatherman_test)
    gettext_templates(${CMAKE_CURRENT_SOURCE_DIR}/locales ${CMAKE_CURRENT_SOURCE_DIR}/tests/locale.cc)
    gettext_compile(${CMAKE_CURRENT_SOURCE_DIR}/locales share/locale)
endif()
