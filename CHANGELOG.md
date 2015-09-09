# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

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
  - time, augmenting Boost.Date_time
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
