include(leatherman)
defoption(COVERALLS "Generate code coverage using Coveralls.io" OFF)
defoption(BOOST_STATIC "Use Boost's static libraries" OFF)
defoption(CURL_STATIC "Use curl's static libraries" OFF)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON CACHE BOOL "Prepend project includes before system includes")
set(LIB_SUFFIX "" CACHE STRING "Library install suffix")

# Solaris and AIX have poor support for std::locale and boost::locale
# with GCC. Don't use them by default.
if (CMAKE_SYSTEM_NAME MATCHES "AIX" OR CMAKE_SYSTEM_NAME MATCHES "SunOS")
    set(USE_BOOST_LOCALE FALSE)
else()
    set(USE_BOOST_LOCALE TRUE)
endif()
defoption(LEATHERMAN_USE_LOCALES "Use locales for internationalization" ${USE_BOOST_LOCALE})

# Provided so it can be disabled temporarily when we don't have gettext built.
defoption(LEATHERMAN_GETTEXT "Support localization with gettext" ON)

# Map our boost option to the for-realsies one
set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
