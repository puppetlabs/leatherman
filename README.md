# Leatherman - a C++ toolkit

## Usage

Leatherman can be used in one of two ways: It can be installed as a
regular library, and included using the normal CMake `find_package`
syntax, or it can be setup as a submodule. The recommended method is
to install Leatherman and use it as a regular system library.

Leatherman is broken up into a number of focused component
libraries. Both methods of using Leatherman allow you to control which
components are built and used.

### Dependencies

* Boost, at least version 1.54

### As a Standalone Library

The recommended way to use Leatherman is as a library built and
installed on your system.

#### Building Leatherman

Leatherman is built like any other cmake project:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

By default, all of the component libraries are built when Leatherman
is used standalone. To disable a component, you can set
`LEATHERMAN_ENABLE_<LIBRARY>` to any of CMake's falsy values.

#### Using Leatherman

Leatherman's `make install` deploys a standard CMake config file to
`lib/cmake/leatherman`. This allows the normal CMake `find_package`
workflow to be used.

    find_package(Leatherman COMPONENTS foo bar baz REQUIRED)

If Leatherman is not installed to a standard system prefix, or on
Windows where there is no standard prefix, you can set
`CMAKE_PREFIX_PATH` to the location of Leatherman's install.

### As a Submodule

Leatherman can be included as a git submodule and added as a CMake
subdirectory. Consider the following:

    CMakeLists.txt
    lib/
        CMakeLists.txt
    vendor/
        leatherman/

In this setup, your CMakeLists.txt would need to contain the following:

    ...
    add_subdirectory(vendor/leatherman)
    ...

To enable individual Leatherman components, you must set
`LEATHERMAN_ENABLE_<LIBRARY>`. Any libraries not explicitly enabled
will not be built or available to the containing project.

    ...
    set(LEATHERMAN_ENABLE_LOCALE TRUE)
    add_subdirectory(vendor/leatherman)
    ...

### Variables Set by Leatherman

Leatherman sets two top-level CMake variables:

* `LEATHERMAN_INCLUDE_DIRS` The include paths of all enabled
  leatherman libraries
* `LEATHERMAN_LIBRARIES` The library names of all enabled leatherman
  libraries, as well as their dependencies.

In addition, each enabled library sets a number of library-specific
variables:

* `LEATHERMAN_<LIBRARY>_INCLUDE` The include directory or directories
  for the given leatherman library.
* `LEATHERMAN_<LIBRARY>_LIB` The library name as used by CMake. In the
  case of header-only leatherman libraries, this will be set to the
  empty string.
* `LEATHERMAN_<LIBRARY>_DEPS` Any dependency libraries needed by the
  given library. This could include other leatherman libraries or
  3rd-party libraries found via CMake.
* `LEATHERMAN_<LIBRARY>_LIBS` The contents of both
  `LEATHERMAN_<LIBRARY>_LIB` and `LEATHERMAN_<LIBRARY>_DEPS`

### CMake Helpers Provided by Leatherman

In addition to the C++ library components, Leatherman provides a few
CMake helpers. These will be automatically added to your
`CMAKE_MODULE_PATH` when `find_package` is processed.

* `options`: Common CMake options for leatherman features. Should
  almost always be used.
* `cflags`: Sets a `LEATHERMAN_CXX_FLAGS` variable containing the
  Puppet Labs standard CXXFLAGS for your compiler and platform.

* `leatherman`: Additional functionality provided by Leatherman for
  consumers. Includes:
  * Helpers for dealing with variables and scopes
  * Debugging macros
  * `cpplint` and `cppcheck` configuration
  * Logging configuration

### Using Logging

Each `.cc` file that uses logging (or includes a header which uses
logging) needs to know its logging namespace. This can be set by
defining `LEATHERMAN_LOGGING_NAMESPACE` to a string such as
"leatherman.logging" or "puppetlabs.facter".

Since typically a large number of files at once will need to use the
same logging namespace, leatherman provides a CMake macro to set it
globally. This can be used as follows:

    ...
    include(leatherman)
    leatherman_logging_namespace("logging.namespace")
    ...

### Using Catch

Since Catch is a testing-only utility, its include directory is
excluded from LEATHERMAN_INCLUDE_DIRS. To use Catch, explicitly add

    include_directories(${LEATHERMAN_CATCH_INCLUDE})

to the CMakeLists.txt file of your testing directory.

## Extending Leatherman

Adding a new library to leatherman is easy!

* Add a new subdirectory with the name of your library
* Add an appropriate `add_leatherman_dir` invocation to the top-level
  `CMakeLists.txt`
* Fill in the headers, sources, and tests of your library. The typical
  directory structure is below.

The `CmakeLists.txt` file for a library is used both at build time and
during a `find_package` call for Leatherman. This allows library
dependencies to be handled identically during both build and find
operations. Because of this, certain build configuration settings
might need to be gated on a check for `BUILDING_LEATHERMAN`. See the
`logging` library for an example of how this is done.

### Typical Leatherman Directory Structure

    leatherman/
        libname/
            CMakeLists.txt
            src/
                srcfile.cc
            inc/leatherman/
                header.hpp
            tests/
                testfile.cc

### Sample Library CMakeLists.txt file

    add_leatherman_library("src/srcfile.cc")
    add_leatherman_test("tests/testfile.cc")
    add_leatherman_headers("inc/leatherman")

More complex libraries may have dependencies. See the `locale` library
for a simple example of how dependencies are handled by leatherman
libraries.

### Vendoring Other Libraries

Sometimes it's necessary to vendor a 3rd-party library in
Leatherman. In these cases the standard Leatherman macros probably
won't help you, and you'll need to write a lower-level CMake
file. This README can't cover all the possible situations here, but
the `nowide` and `catch` CMake files are both solid examples.
