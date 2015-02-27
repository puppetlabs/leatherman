find_package(Boost 1.54 REQUIRED COMPONENTS log)

include_directories(${Boost_INCLUDE_DIRS})
add_leatherman_deps("${Boost_LOG_LIBRARY}")

leatherman_dependency(nowide)
leatherman_dependency(locale)

leatherman_logging_namespace("leatherman.logging")
add_leatherman_library(src/logging.cc)
add_leatherman_test(tests/logging.cc)
