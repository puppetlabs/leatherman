set(${include_var} "${PROJECT_SOURCE_DIR}/vendor/boost-nowide" PARENT_SCOPE)
if (WIN32)
    set(${lib_var} nowide-shared PARENT_SCOPE)
else()
    set(BOOST_NOWIDE_SKIP_TESTS ON CACHE BOOL "Disable tests in Boost.Nowide")
endif()

# Disable installing Boost.Nowide so we don't pollute the installation.
set(BOOST_NOWIDE_SKIP_INSTALL ON CACHE BOOL "Disable installing Boost.Nowide")

# have to specify a bindir since this isn't actually a subdirectory of us
add_subdirectory("../vendor/boost-nowide" "../vendor/boost-nowide")
