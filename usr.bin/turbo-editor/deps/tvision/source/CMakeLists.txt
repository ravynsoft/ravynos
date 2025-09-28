# Turbo Vision sources.

file(GLOB_RECURSE TVSOURCE "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp")
list(REMOVE_ITEM TVSOURCE "${CMAKE_CURRENT_LIST_DIR}/tvision/geninc.cpp")

add_library(${PROJECT_NAME} STATIC ${TVSOURCE})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

tv_add_private_includes(${PROJECT_NAME})
tv_set_warnings(${PROJECT_NAME})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PUBLIC
        /permissive-
        /Zc:__cplusplus
        /Zc:externConstexpr
        /Zc:inline
    )
    if (TV_REDUCE_APP_SIZE) # Implies CMake >= 3.13.0
        target_compile_options(${PROJECT_NAME} PRIVATE
            /Gy
        )
        target_link_options(${PROJECT_NAME} PUBLIC
            /OPT:REF
        )
    endif()
else()
    if (TV_REDUCE_APP_SIZE) # Implies CMake >= 3.13.0
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fdata-sections -ffunction-sections
        )
        target_link_options(${PROJECT_NAME} PUBLIC
            -Wl,--gc-sections
        )
    endif()
endif()

foreach (cfg ${CMAKE_CONFIGURATION_TYPES})
    # Ensure all configurations produce different library names,
    # so that they can be installed together.
    string(TOUPPER ${cfg} cfg_up)
    string(TOLOWER ${cfg} cfg_low)
    if (NOT (cfg_up STREQUAL "RELEASE"))
        set_target_properties(${PROJECT_NAME} PROPERTIES ${cfg_up}_POSTFIX -${cfg_low})
    endif()
endforeach()

if (TV_USE_STATIC_RTL) # Implies MSVC
    target_compile_options(${PROJECT_NAME} PUBLIC
        /MT$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:d>
    )
endif()

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        _CRT_NONSTDC_NO_WARNINGS
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        # Windows Vista is the minimum supported version.
        _WIN32_WINNT=0x0600
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Do not include unnecessary STL headers to speed up compilation.
    TVISION_NO_STL
)

include(TestBigEndian)

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (IS_BIG_ENDIAN)
    tv_message(STATUS "Big endian system detected")
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        TV_BIG_ENDIAN
    )
endif()

# Dependencies

if (NOT WIN32)
    # ncursesw
    find_library(NCURSESW ncursesw)
    if (NOT NCURSESW AND APPLE)
        # macOS has no ncursesw by default
        find_library(NCURSESW ncurses)
    endif()
    if (NOT NCURSESW)
        tv_message(FATAL_ERROR "Library 'ncursesw' not found")
    else()
        tv_message(STATUS "Found 'ncursesw': ${NCURSESW}")
    endif()
    # Some distributions place ncurses.h under a separate directory.
    find_path(NCURSESW_INCLUDE "ncursesw/ncurses.h")
    if (NCURSESW_INCLUDE)
        target_include_directories(${PROJECT_NAME} PRIVATE "${NCURSESW_INCLUDE}/ncursesw")
    else()
        find_path(NCURSESW_INCLUDE "ncurses.h")
        if (NCURSESW_INCLUDE)
            target_include_directories(${PROJECT_NAME} PRIVATE "${NCURSESW_INCLUDE}")
        else()
            tv_message(FATAL_ERROR "'ncursesw' development headers not found")
        endif()
    endif()

    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_NCURSES)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${NCURSESW})

    # tinfow, dependency of ncurses. Often provided as 'tinfo', but we also need
    # to link the 'w' version when both are available.
    find_library(TINFOW tinfow)
    if (NOT TINFOW)
        find_library(TINFOW tinfo)
    endif()
    if (TINFOW)
        tv_message(STATUS "Found 'tinfow': ${TINFOW}")
        target_link_libraries(${PROJECT_NAME} PUBLIC ${TINFOW})
    endif()

    # gpm
    if (TV_BUILD_USING_GPM)
        find_library(GPM gpm)
        if (NOT GPM)
            tv_message(WARNING "'gpm' library requested but not found")
        else()
            tv_message(STATUS "Found 'gpm': ${GPM}")
            find_path(GPM_INCLUDE "gpm.h")
            if (GPM_INCLUDE)
                target_include_directories(${PROJECT_NAME} PRIVATE "${GPM_INCLUDE}")
            else()
                tv_message(FATAL_ERROR "'gpm' development headers not found")
            endif()
            target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GPM)
            target_link_libraries(${PROJECT_NAME} PUBLIC ${GPM})
        endif()
    endif()
endif()

tv_set_output_dir(${PROJECT_NAME})

# allow CMAKE_INSTALL_PREFIX to decide final install position
#
# static lib
#
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-config
    ARCHIVE DESTINATION lib
    COMPONENT library
)

# package configuration

install(EXPORT ${PROJECT_NAME}-config
    DESTINATION lib/cmake/${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}-config.cmake
    COMPONENT library
)

# includes
# ./include/tvision and children copied to destination/include/tvision etc...
#
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/tvision" DESTINATION include)

# Build optimization

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0" AND TV_OPTIMIZE_BUILD)
    # Enable precompiled headers.
    target_precompile_headers(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include/pch.h")

    if (TV_LIBRARY_UNITY_BUILD)
        # Enable Unity Build.
        # Files with non-trivial global variables must not be included in Unity Build.
        file(GLOB_RECURSE TVSOURCE_NOUNITY "${CMAKE_CURRENT_LIST_DIR}/tvision/s*.cpp")
        list(APPEND TVSOURCE_NOUNITY "${CMAKE_CURRENT_LIST_DIR}/tvision/new.cpp")
        list(REMOVE_ITEM TVSOURCE_NOUNITY
            "${CMAKE_CURRENT_LIST_DIR}/tvision/snprintf.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/tvision/stddlg.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/tvision/strmstat.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/tvision/syserr.cpp"
        )
        set_property(SOURCE ${TVSOURCE_NOUNITY} PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)
        set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
    endif()
endif()
