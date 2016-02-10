include(leatherman)
defoption(COVERALLS "Generate code coverage using Coveralls.io" OFF)
defoption(BOOST_STATIC "Use Boost's static libraries" OFF)
defoption(CURL_STATIC "Use curl's static libraries" OFF)
set(LIB_SUFFIX "" CACHE STRING "Library install suffix")

# Solaris and AIX have poor support for std::locale and boost::locale
# with GCC. Don't use them by default.
if (CMAKE_SYSTEM_NAME MATCHES "AIX" OR CMAKE_SYSTEM_NAME MATCHES "SunOS")
    set(USE_BOOST_LOCALE FALSE)
else()
    set(USE_BOOST_LOCALE TRUE)
endif()
defoption(LEATHERMAN_ENABLE_LOCALE "Use locales for internationalization" ${USE_BOOST_LOCALE})

# Map our boost option to the for-realsies one
set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
