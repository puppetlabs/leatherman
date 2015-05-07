if (BUILDING_LEATHERMAN)
    # Just point the user at our vendored catch. Nothing else to do here
    set(include_dir "${PROJECT_SOURCE_DIR}/vendor/catch/include")
    set(${include_var} ${include_dir} PARENT_SCOPE)

    # Because we're weird and vendored, we need to bypass the normal
    # leatherman install helpers.
    if (LEATHERMAN_INSTALL)
        install(FILES "${include_dir}/catch.hpp" DESTINATION "include/leatherman/vendor/catch")
    endif()
else()
    set(${include_var} "${LEATHERMAN_PREFIX}/include/leatherman/vendor/catch")
endif()