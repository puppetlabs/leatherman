if (BUILDING_LEATHERMAN)
    set(${include_var} "${PROJECT_SOURCE_DIR}/vendor/boost-nowide" PARENT_SCOPE)
    if (WIN32)
        set(${lib_var} nowide-shared PARENT_SCOPE)
    endif()

    set(BOOST_NOWIDE_SKIP_TESTS ON CACHE BOOL "Disable tests in Boost.Nowide")

    # Disable installing Boost.Nowide so we don't pollute the
    # installation. We will handle installing the appropriate header files
    # to our own vendor include dir.
    set(BOOST_NOWIDE_SKIP_INSTALL ON CACHE BOOL "Disable installing Boost.Nowide")

    # have to specify a bindir since this isn't actually a subdirectory of us
    set(srcdir "../vendor/boost-nowide")
    set(bindir "${CMAKE_CURRENT_BINARY_DIR}/${srcdir}")
    add_subdirectory(${srcdir} ${bindir})

    if (LEATHERMAN_INSTALL)
        install(DIRECTORY "${srcdir}/boost" DESTINATION "include/leatherman/vendor/boost-nowide")

        if (WIN32)
            # Have to install as FILES instead of TARGETS because the
            # target isn't in this CMake file
            get_target_property(output_name nowide-shared OUTPUT_NAME)
            install(FILES "${bindir}/${CMAKE_IMPORT_LIBRARY_PREFIX}${output_name}${CMAKE_IMPORT_LIBRARY_SUFFIX}" DESTINATION "lib/leatherman")
            install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}${output_name}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION "bin")
        endif()
    endif()
else()
    set(${include_var} "${LEATHERMAN_PREFIX}/include/leatherman/vendor/boost-nowide")

    if (WIN32)
        set(${lib_var} nowide-shared)
        add_library(nowide-shared SHARED IMPORTED)
        find_library( NOWIDE_LIB
            NAMES libnowide
            PATHS "${LEATHERMAN_PREFIX}/lib/leatherman"
            NO_DEFAULT_PATH
            )
        set_target_properties(nowide-shared PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            IMPORTED_LOCATION "${LEATHERMAN_PREFIX}/bin/libnowide.dll"
            IMPORTED_IMPLIB ${NOWIDE_LIB}
        )
    endif()
endif()
