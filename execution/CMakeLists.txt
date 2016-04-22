find_package(Boost 1.54 REQUIRED COMPONENTS regex filesystem system)

add_leatherman_deps("${Boost_LIBRARIES}")
add_leatherman_includes("${Boost_INCLUDE_DIRS}")

leatherman_dependency(util)
leatherman_dependency(nowide)
leatherman_dependency(locale)
leatherman_dependency(logging)
leatherman_dependency(file_util)
if (BUILDING_LEATHERMAN)
    leatherman_logging_namespace("leatherman.execution")
    leatherman_logging_line_numbers()
endif()

if(WIN32)
    leatherman_dependency(windows)
endif()

add_leatherman_headers(inc/leatherman)
if(WIN32)
    add_leatherman_library(src/execution.cc src/windows/execution.cc)
else()
    add_leatherman_library(src/execution.cc src/posix/execution.cc)
endif()

if(WIN32)
    set(PLATFORM_TESTS tests/windows/execution.cc)
else()
    set(PLATFORM_TESTS tests/posix/execution.cc)
endif()

add_leatherman_test(tests/log_capture.cc ${PLATFORM_TESTS})

if (BUILDING_LEATHERMAN)
    # Dumb implementation of cat.exe for testing stdin/stdout/stderr handling.
    include_directories(BEFORE ${LEATHERMAN_NOWIDE_INCLUDE})
    add_executable(lth_cat tests/lth_cat.cc)
    target_link_libraries(lth_cat ${LEATHERMAN_NOWIDE_LIBS})
    if (COVERALLS)
        target_link_libraries(lth_cat gcov)
    endif()
    set_target_properties(lth_cat PROPERTIES COMPILE_FLAGS "${LEATHERMAN_CXX_FLAGS}")

    configure_file (
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp.in"
        "${CMAKE_CURRENT_LIST_DIR}/tests/fixtures.hpp"
    )
endif()
