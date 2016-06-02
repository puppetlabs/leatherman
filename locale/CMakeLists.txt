if (LEATHERMAN_USE_LOCALES)
    find_package(Boost 1.54 REQUIRED COMPONENTS locale system)
else()
    find_package(Boost 1.54 REQUIRED regex)
endif()

add_leatherman_includes(${Boost_INCLUDE_DIRS})
add_leatherman_deps(${Boost_LIBRARIES})

if (LEATHERMAN_USE_LOCALES AND BOOST_STATIC AND APPLE)
    # Boost.Locale relies on libiconv; if not using shared boost libraries
    # we need to include the dependency ourselves. So far this is only a
    # problem on Mac OS X.
    add_leatherman_deps(iconv)
endif()

add_leatherman_headers(inc/leatherman)

if (LEATHERMAN_USE_LOCALES)
    add_leatherman_library(src/locale.cc)
    if (GETTEXT_ENABLED)
        # This test relies on translation .mo files being generated.
        # Projects that don't support localization yet still need
        # tests to pass, so only enable these tests if gettext is
        # available.
        add_leatherman_test(tests/locale.cc)
    endif()
else()
    add_leatherman_library(disabled/locale.cc)
endif()

add_leatherman_test(tests/format.cc)

if (LEATHERMAN_USE_LOCALES AND BUILDING_LEATHERMAN)
    project(leatherman_locale)
    add_subdirectory(locales)
endif()
