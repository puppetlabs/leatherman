# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [1.4.0]

### Changed
- Updated Catch library to 1.10.0.
- Updated Boost.Nowide to ec9672b
- Update Travis CI to use container-based builds

### Fixed
- Builds with Xcode 9
- Allow Leatherman.execute calls to opt into allowing tasks to finish without reading stdin (i.e. don't fail when the pipe is closed) via the `allow_stdin_unread` option. This specifically supports pxp-agent's task execution where input may or may not be used. (LTH-149)

## [1.3.0]

### Added
- A toPrettyJson routine to Leatherman.json\_container that pretty prints a valid JSON object.

## [1.2.2]

### Fixed
- Allow Leatherman.execute calls to opt into allowing tasks to finish without reading stdin (i.e. don't fail when the pipe is closed) via the `allow_stdin_unread` option. This specifically supports pxp-agent's task execution where input may or may not be used. (LTH-149)

## [1.2.1]

### Fixed
- Made Leatherman.curl's download_file response accessible, with results included when an error code is returned.

## [1.2.0]

### Added
- A URI parsing utility in Leatherman.util (LTH-143)

### Changed
- Refactored Leatherman.curl's http_file_operation_exception to inherit from http_request_exception so that the three possible failure modes are distinguished by class

## [1.1.2]

### Fixed
- Separated the possible failure modes for Leatherman.curl into three categories (LTH-142)
     * Curl setup errors
     * File operation errors (e.g. writing to disk during file download)
     * Server side errors (e.g. bad host)

- Simplified the logic of Leatherman.curl's download_file method by abstracting out actions associated with the temporary file (creating, writing, removing) to a class that follows the RAII pattern. (LTH-142)

## [1.1.1]

### Fixed
- Generate build artifact with GCC 5.2.0 on Windows.

## [1.1.0]

### Added
- Execution with file redirection and `atomic_write_to_file` can specify the permissions of those files. (LTH-139)
- Leatherman.curl added a download_file function for doing streaming file downloads. (LTH-140)

### Fixed
- Fix redundant newlines when using `execute` that redirects output to files when not using the `trim` option. This combination now also ensures empty lines are not skipped. A side effect is that when not using `trim`, empty lines may appear when iterating over lines of output via `each_line` as well. (LTH-138)

## [1.0.0]
Final tag for Leatherman 1.0.0, containing the same change set as 0.99.0.

## [0.99.0]
This is a pre-release version for Leatherman 1.0.0, containing backwards-incompatible API changes.

### Changed
- Remove Ruby bindings for Fixnum and Bignum, replace with Integer for Ruby 2.4 support (LTH-124)

## [0.12.1]

### Fixed
- Locale files are installed relative to the Leatherman install root, taking into account support for relocatable packages (LTH-135)

### Added
- Ruby API binding for rb_ll2inum (missing from 0.12.0)

## [0.12.0]

### Added
- Support for finding locale files with a relocatable package, particularly on Windows (LTH-133)

## [0.11.2]

### Added
- Ruby API binding for rb_ll2inum

## [0.11.1]

### Fixed
- Circumvent a bug in Boost.Log's severity logger that prevented logging on AIX (LTH-128)

## [0.11.0]

### Added
- Add an option to use thread-safe forking at the expense of failing to fork if maxed out on memory; more permanent fix for the Solaris deadlock issue addressed in 0.10.2 (LTH-126)

## [0.10.2]

### Fixed
- Avoid deadlock on Solaris using vfork/exec in a multithreaded process with Leatherman.execution (LTH-125)

## [0.10.1]

### Fixed
- Only apply large file support flags to Leatherman.execution (LTH-120)

## [0.10.0]

### Added
- Solaris implementation of the `create_detached_process` execution option - execution of child processes in separate contracts (LTH-120)

### Changed
- Renamed `create_new_process_group` execution option to `create_detached_process` to make the concept more broadly applicable (LTH-120)

## [0.9.4]

### Fixed
- Handle null characters in JSON strings (LTH-116)
- Explicitly pass release flag to pod2man

## [0.9.3]

### Fixed
- Fix Boost.Log sink initialization with Boost 1.62 (LTH-115)

### Changed
- Switch to compile-time unpacking vendored packages (LTH-117)

## [0.9.2]

### Changed
- leatherman::execution can now be requested to convert Windows line endings to standard ones (LTH-114)

## [0.9.1]

### Fixed
- Externalized some strings for localization that were missed (LTH-59)
- Fixed consuming Leatherman without cflags (LTH-113)
- Updated the logging backend to filter records based on log level; previously filtering was only applied when using Leatherman logging functions.

### Changed
- Third-party libraries are now added as compressed files

## [0.9.0]

### Added
- Add translation helper functions and plural format support (LTH-109)

## [0.8.1]

### Fixed
- Fix compilation with curl 7.50.0

## [0.8.0]

### Added
- Added protect version number to libraries

### Fixed
- Fix Leatherman cleanup of Ruby objects (FACT-1454)
- Add inherit_locale option to execute (LTH-107)

## [0.7.5]

### Added
- Added `leatherman::windows::file_util::get_programdata_dir` to properly get
  the ProgramData directory on Windows.

### Fixed
- Changed the windows logging namespace to logging.windows.

## [0.7.4]

### Fixed
- Leatherman.Ruby compatibility with Ruby 2.3.

## [0.7.3]

### Fixed
- Fixed compilation with LEATHERMAN_USE_LOCALES=OFF.
- Remove line numbers from .pot files generated via gettext.

### Added
- Added LEATHERMAN_GETTEXT option to disable use of gettext.

## [0.7.2]

### Fixed
- Fallback to multi-threaded apartments for COM on Microsoft Nano Server.

### Added
- Add `Util::Timer::elapsed_milliseconds`.
- Add context and plural support in Leatherman.Locale `translate` (and new `translate_c`) methods.

## [0.7.1]

### Fixed
- Binary compatibility with 0.6.x has been restored

## [0.7.0]

### Fixed
- `symbol_exports` helper no longer applies its macros to all targets

### Added
- (LTH-97) Applications can now disable locale support in logging

## [0.6.3]

### Fixed
- (LTH-96) Translate log message without substitutions
- (LTH-95) Fix unit tests using shared libraries on Windows
- Minor updates to tests and documentation

## [0.6.2] - 2016-04-20

### Fixed
- Runtime shared library load errors on AIX.

## [0.6.1] - 2016-04-19

### Fixed
- Missing include header in leatherman/ruby/api.h, needed on Mac OS X

## [0.6.0] - 2016-04-19

### Fixed
- Outputting WMI errors when l10n is disabled
- Leatherman will no longer use installed leatherman headers when building itself

### Added
- Leatherman now builds on Windows Nano Server
- Ruby binding for `rb_last_status_set`, needed for Facter's execution API
- The `result` struct in `execution` now contains the PID of the executed processes

## [0.5.1] - 2016-04-18

0.5.0 was incorrectly tagged, causing Travis and Appveyor to skip creating build artifacts.

## [0.5.0] - 2016-04-18

### Fixed
- Static dependency libraries will no longer be linked to Leatherman consumers when using a shared leatherman library
- Interfacing with ruby APIs for 64-bit integers on Windows (See Added and Removed below for details)

### Added
- Ability to spawn child processes in a new group on Windows
- Ruby bindings for `is_bignum`
- Ruby `num2size_t` help for consistent access to array/string sizes

### Removed
- Windows 2003 / XP Support. This allows us to better take advantage of modern Windows APIs and features
- Bindings to ruby `rb_num2long` and `rb_num2ulong`, as they were inconsistent across platforms

## [0.4.2] - 2016-03-07

### Fixed
- `find_package(Leatherman)` will now raise a CMake error if a consuming application requests locale support when leatherman was built without it.

### Added
- A preprocessor definition `LEATHERMAN_USE_LOCALES` for consuming projects to know whether locale support is enabled.

## [0.4.1] - 2016-03-02

### Fixed
- Install `generate_translations.cmake` for internationalization
- Fix builds on Mac OS X against static boost

## [0.4.0] - 2016-02-23

### Fixed

- Header search order when Leatherman is installed to a default system path
- Ruby string conversion when the Ruby string is in a non-unicode locale
- Link order when building a shared library on Windows

### Added

- i18n support using Boost::Locale and gettext .po files

## [0.3.7] - 2016-02-10

### Fixed

- Made the pod2man CMake macro available to downstream consumers.

## [0.3.6] - 2016-02-05

### Fixed

- Added version to Leatherman CMake config, so downstream projects can depend on a particular version.

### Added

- Added pod2man macro for generating man pages.

## [0.3.5] - 2016-01-14

### Fixed

- `leatherman.ruby` can now find a Ruby DLL on Windows when Leatherman is compiled as shared libraries (LTH-71)
- `leatherman.dynamic_library` debug logging when searching for a library will now correctly print the name of the library
- Leatherman unit tests will now run successfully under Cygwin

## [0.3.4] - 2015-12-29

### Fixed

- Fixed a compilation issue with the execution tests on OSX.

## [0.3.3] - 2015-12-23

### Changed

- The vendored `boost::nowide` has been updated to a version that supports C++11 iostream changes
- `LIB_SUFFIX` is now respected for installing to `lib32` or `lib64` if needed

### Fixed

- It is now possible to buld Leatherman without curl support
- It is now possible to build leatherman as a set of DLLs on Windows
- An order-dependant unit test issue has been resolved

### Known Issues

- Leatherman cannot load ruby when built as a DLL on Windows (LTH-71)

## [0.3.2] - 2015-12-16

### Fixed

- The `windows` library incorrectly used `target_link_libraries` instead of `add_leatherman_deps`

## [0.3.1] - 2015-12-16

### Fixed

- The key for publishing builds from travis was incorrectly encrypted.

## [0.3.0] - 2015-12-16

### Added
- Option to build dynamic libraries
- `leatherman_install` helper for installing targets consistently
- leatherman.ruby
  - added rb\_num2long and rb\_cBignum to the API
  - add array\_len to query the length of a Ruby array
- leatherman.execution
  - return exit\_code from execute commands, and switch to returning a struct with named members
  - add child process stdin support to execute
  - add execute() overloads for registering a callback for the PID (once known), and redirecting streams to files
- leatherman.util - add scoped\_handle helper

### Changed
- Updated cpplint to version `#409`
- Use static libnowide by default
- leatherman.curl
  - added const annotations to curl::response::header
- leatherman.json\_container
  - removed unnecessary vector copying

### Fixed
- Builds on AIX
- Builds with Xcode 7
- Builds with GCC 5.2
- Fixed using as a stand-alone library
- leatherman.curl
  - support redirects; added seek\_body to specify a seek function, and set\_body now requires specifying the http\_method
- leatherman.execution
  - fixed occasionally skipping final output from stderr
  - protect against potential named pipe re-use
- leatherman.logging - fix error message when requesting an invalid log\_level to correctly show the requested level
- leatherman.json\_container
  - remove use of MemoryPoolAllocator with rapidjson, as it's buggy on Solaris SPARC
  - fixed freed memory read with getRaw

### Known Issues
- Dynamic library builds fail on Windows because dllexports aren't declared

## [0.2.0] - 2015-09-09

### Added
- leatherman.curl - a C++ interface for libcurl
- leatherman.dynamic\_library - cross-platform loading of dynamic libraries
- leatherman.execution - cross-platform system invocation with input/output support
- leatherman.file\_util - utilities for manipulating files, augmenting Boost.FileSystem
- leatherman.json\_container - a simplified C++ interface for rapidjson 
- leatherman.ruby - support for embedding and working with the Ruby interpreter
- leatherman.util - general C++ utilities
  - strings, augmenting Boost.Algorithms
  - time, augmenting Boost.Date\_time
  - RAII wrappers
  - environment variables
  - regex helpers, augmenting Boost.Regex
- leatherman.windows - Windows-specific C++ utilities
  - process and user querying
  - registry queries
  - error wrapper
  - wmi queries
- the rapidjson library
- CMake utilities
  - get git revision
  - put binaries in one directory on Windows
  - link against CURL statically or dynamically

### Changed
- Leatherman can now be installed to the system as a stand-alone library.
- Logging can now include source line numbers, enabled with the CMake macro `leatherman_logging_line_numbers()`.
- Logging on Windows now includes colored output

### Fixed
- Now links the correct Boost optimized/debug libraries.
- Builds on Solaris and FreeBSD
- Remove CMake requirement that Ruby is installed. Leatherman.ruby tests will still expect Ruby and fail if missing.
- CMake no longer errors if including header-only libraries such as Catch.

## [0.1.0] - 2015-06-16

### Added
- leatherman.locale - set locale across platforms
- leatherman.logging - logging based on Boost.Log
- CMake utilities
  - compile flags
  - Coveralls.io setup
  - link Boost statically or dynamically
- the Catch C++ testing framework
- the Boost.nowide library for cross-platform UTF-8 io
