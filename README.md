# Leatherman - a C++ toolkit

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Usage](#usage)
  - [Dependencies](#dependencies)
  - [As a Standalone Library](#as-a-standalone-library)
    - [Building Leatherman](#building-leatherman)
    - [Using Leatherman](#using-leatherman)
  - [As a Submodule](#as-a-submodule)
  - [Variables Set by Leatherman](#variables-set-by-leatherman)
  - [CMake Helpers Provided by Leatherman](#cmake-helpers-provided-by-leatherman)
  - [Internationalization (i18n)](#internationalization-i18n)
  - [Using Logging](#using-logging)
  - [Using Catch](#using-catch)
  - [Using Windows](#using-windows)
  - [Using JsonContainer](#using-jsoncontainer)
  - [Using curl](#using-curl)
- [Extending Leatherman](#extending-leatherman)
  - [Typical Leatherman Directory Structure](#typical-leatherman-directory-structure)
  - [Sample Library CMakeLists.txt file](#sample-library-cmakeliststxt-file)
  - [Vendoring Other Libraries](#vendoring-other-libraries)
- [How To Release](#how-to-release)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Usage

Leatherman can be used in one of two ways: It can be installed as a
regular library, and included using the normal CMake `find_package`
syntax, or it can be setup as a submodule. The recommended method is
to install Leatherman and use it as a regular system library.

Leatherman is broken up into a number of focused component
libraries. Both methods of using Leatherman allow you to control which
components are built and used.

Library install locations can be controlled using the LIB_SUFFIX
variable, which results in installing libraries to `lib${LIB_SUFFIX}`.

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
  * Install command with cross-platform defaults
  * Symbol visibility configuration

* `pod2man`: Adds a `pod2man` macro to generate man files from source.

### Internationalization (i18n)

Leatherman and its components provide support for generating and using
`gettext`-based message catalogs. Two helpers are provided for
generating message catalogs:

* `gettext_templates <dir> <sources>`: creates a `${PROJECT_NAME}.pot`
  target (used by `all`) that (re)generates the .pot file from specified
  source files. If the project is configured with `LEATHERMAN_LOCALES`
  containing a list of language codes, it will add a target
  `${PROJECT_NAME}-${LANG}.po` to create or update translation (.po)
  files matching those codes. Files are put in `dir`.
* `gettext_compile <dir> <inst>`: creates a `translation` target (also
  used by `all`) to generate the binary message catalogs (.mo files) and
  configure installing them to the specified install location (`inst`).

`LEATHERMAN_LOCALES` expects a quoted semi-colon separated list, as
in `LEATHERMAN_LOCALES="en;fr;ja"`.

Normal use of cmake/make should ensure the translation files are up-to-
date. Translations can be tested by setting the `LC_CTYPE` environment
variable.

By default i18n support is disabled. To enable it, define `LEATHERMAN_I18N`
when compiling your project.

To translate strings outside of logging, use the
`leatherman::locale::translate` and `leatherman::locale::format`
helpers. Strings passed to the helpers will be extracted to .po files.
`leatherman::locale::format` is a drop-in replacement for
[`boost::locale::format`](http://www.boost.org/doc/libs/1_58_0/libs/locale/doc/html/localized_text_formatting.html),
which adds locale-aware formatting to `boost::format`, but requires
different substitution tokens. To support transparently enabling
`LEATHERMAN_I18N` for only some platforms in a project,
`leatherman::locale::format` falls-back to using `boost::format`, and
will convert substitution tokens using the regex `{(\d+)}` to `%\1%`.
To be safe, assume both formats are special when using `format`, and
use `{N}` in as the substitution token for your strings.

Translation isn't supported on AIX or Solaris, as GCC on those platforms
doesn't support `std::locale`. In fact std::locale is buggy, so avoid
using `get_locale` as well.

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

Initializing logging via setup\_logging will configure the ostream
for the default UTF-8 locale (or the specified locale).

### Using Catch

Since [Catch][1] is a testing-only utility, its include directory is
excluded from LEATHERMAN\_INCLUDE\_DIRS. To use Catch, explicitly add

    include_directories(${LEATHERMAN_CATCH_INCLUDE})

to the CMakeLists.txt file of your testing directory.

### Using Windows

In order to use the Windows libraries, Logging must be set up.

### Using JsonContainer

To use JsonContainer, you must enable [RapidJSON][2] that is included
as a leatherman component.
Please refer to the [JsonContainer documentation][3] for API details.

### Using curl

To use the curl wrapper library, libcurl must be installed.

On Ubuntu use the following:

    apt-get install libcurl4-openssl-dev
    
On Windows, in Powershell, use:

    (New-Object net.webclient).DownloadFile("http://curl.haxx.se/download/curl-7.42.1.zip", "C:\tools\curl-7.42.1.zip")
    & 7za x "curl-7.42.1.zip" | FIND /V "ing "
    cd curl-7.42.1
    mkdir -Path C:\tools\curl-7.42.1-x86_64_mingw-w64_4.8.4_win32_seh\include
    cp -r include\curl C:\tools\curl-7.42.1-x86_64_mingw-w64_4.8.4_win32_seh\include
    mkdir -Path C:\tools\curl-7.42.1-x86_64_mingw-w64_4.8.4_win32_seh\lib
    cp lib\libcurl.a C:\tools\curl-7.42.1-x86_64_mingw-w64_4.8.4_win32_seh\lib
    
On Windows CMake must also be manually pointed to the correct directory by passing the argument 
`-DCMAKE_PREFIX_PATH="C:\tools\curl-7.42.1-x86_64_mingw-w64_4.8.4_win32_seh`.

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

## How To Release

1. Update [CHANGELOG.md](CHANGELOG.md) with release notes based on
``git log `git describe --abbrev=0 --tags`..HEAD``
1. Update the version in the project declaration of [CMakeLists.txt](CMakeLists.txt)
1. `git tag -s <version> -m '<version>' && git push <puppetlabs> refs/tags/<version>`

[1]: https://github.com/philsquared/Catch
[2]: https://github.com/miloyip/rapidjson
[3]: json_container/README.md
