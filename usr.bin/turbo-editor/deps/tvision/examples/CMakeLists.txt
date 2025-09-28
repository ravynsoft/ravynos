if (TV_BUILD_EXAMPLES)

    function(tv_app_enable_pch app)
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0")
            if (app STREQUAL "hello")
                target_precompile_headers(hello PRIVATE "${PROJECT_SOURCE_DIR}/include/pch.h")
            else()
                target_precompile_headers(${app} REUSE_FROM hello)
            endif()
        endif()
    endfunction()

    function(tv_app_optimize app)
        if (TV_OPTIMIZE_BUILD)
            tv_app_enable_pch(${app})
            tv_enable_unity(${app})
        endif()
    endfunction()

    function(tv_app_common app)
        tv_set_output_dir(${app})
        tv_add_private_includes(${app})
        tv_set_warnings(${app})
        tv_app_optimize(${app})
        # Until CMake 3.13, 'install' only accepts targets defined
        # in the current directory. So install from this function.
        if (${app} IN_LIST TVINSTALLAPPS)
            install(TARGETS ${app} RUNTIME DESTINATION bin)
        endif()
    endfunction()

    function(tv_app_add_generic app)
        file(GLOB_RECURSE app-src "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
        add_executable(${app} ${app-src})
        target_link_libraries(${app} PUBLIC ${PROJECT_NAME})
        tv_app_common(${app})
    endfunction()

    list(APPEND TVAPPS tvedit tvdemo tvdir tvforms tvhc mmenu palette)
    list(APPEND TVINSTALLAPPS tvedit tvdemo tvhc)

    add_executable(hello "${PROJECT_SOURCE_DIR}/hello.cpp")
    target_link_libraries(hello PUBLIC ${PROJECT_NAME})
    tv_app_common(hello)

    foreach(app ${TVAPPS})
        add_subdirectory(${app})
    endforeach()

endif()

add_subdirectory(avscolor)
