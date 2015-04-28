find_package(Boost 1.54 REQUIRED COMPONENTS system filesystem thread date_time log chrono)

add_leatherman_deps("${Boost_SYSTEM_LIBRARY}" "${Boost_FILESYSTEM_LIBRARY}" "${Boost_THREAD_LIBRARY}" "${Boost_DATE_TIME_LIBRARY}" "${Boost_LOG_LIBRARY}" "${Boost_CHRONO}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(nowide)
leatherman_dependency(locale)

if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.logging")
    leatherman_logging_line_numbers()
endif()

add_leatherman_library(src/logging.cc)
add_leatherman_test(tests/logging.cc tests/logging_lines.cc)
add_leatherman_headers(inc/leatherman)
