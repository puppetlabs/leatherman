# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

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
