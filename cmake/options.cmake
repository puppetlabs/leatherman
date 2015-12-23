include(leatherman)
defoption(COVERALLS "Generate code coverage using Coveralls.io" OFF)
defoption(BOOST_STATIC "Use Boost's static libraries" OFF)
defoption(CURL_STATIC "Use curl's static libraries" OFF)
set(LIB_SUFFIX "" CACHE STRING "Library install suffix")

# Map our boost option to the for-realsies one
set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
