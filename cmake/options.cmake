include(leatherman)
defoption(COVERALLS "Generate code coverage using Coveralls.io" OFF)
defoption(BOOST_STATIC "Use Boost's static libraries" OFF)

# Map our boost option to the for-realsies one
set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
