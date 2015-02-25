# Leatherman - a C++ toolkit

## Usage

Leatherman is intended to be included as a git submodule and added as
a CMake subdirectory. Consider the following:

    CMakeLists.txt
    lib/
        CMakeLists.txt
    vendor/
        leatherman/

In this setup, your CMakeLists.txt would need to contain the following:

    ...
    add_subdirectory(vendor/leatherman)
    ...

### Enabling leatherman components

Leatherman is broken up into a number of small, focused
libraries. Each of these can be individually enabled with
`LEATHERMAN_ENABLE_<LIBRARY>`. Any libraries not explicitly enabled
will not be built or available to the containing project.

    ...
    set(LEATHERMAN_ENABLE_LOCALE TRUE)
    add_subdirectory(vendor/leatherman)
    ...

### Variables set by leatherman

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

## Extending leatherman

Adding a new library to leatherman is easy!

* Add a new subdirectory with the name of your library
* Add an appropriate `add_leatherman_dir` invocation to the top-level
  CMakeLists.txt
* Fill in the headers, sources, and tests of your library. The typical
  directory structure is below.

### Typical leatherman directory structure

    leatherman/
        libname/
            CMakeLists.txt
            src/
                srcfile.cc
            inc/
                header.hpp
            tests/
                testfile.cc

### Sample library CMakeLists.txt file

    add_leatherman_library("src/srcfile.cc")
    add_leatherman_test("tests/testfile.cc")

More complex libraries may have dependencies. See the `locale` library
for an example of how dependencies are handled by leatherman
libraries.
