cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(qtermwidget)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)
include(CheckIncludeFile)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(BUILD_EXAMPLE "Build example application. Default OFF." OFF)
option(QTERMWIDGET_USE_UTEMPTER "Uses libutempter on Linux or libulog on FreeBSD for login records." OFF)
option(QTERMWIDGET_BUILD_PYTHON_BINDING "Build python binding" OFF)
option(USE_UTF8PROC "Use libutf8proc for better Unicode support. Default OFF" OFF)


# just change version for releases
set(QTERMWIDGET_VERSION_MAJOR "0")
set(QTERMWIDGET_VERSION_MINOR "17")
set(QTERMWIDGET_VERSION_PATCH "0")

set(QTERMWIDGET_VERSION "${QTERMWIDGET_VERSION_MAJOR}.${QTERMWIDGET_VERSION_MINOR}.${QTERMWIDGET_VERSION_PATCH}")

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Minimum Versions
set(QT_MINIMUM_VERSION "5.12.0")
set(LXQTBT_MINIMUM_VERSION "0.9.0")

find_package(Qt5Widgets "${QT_MINIMUM_VERSION}" REQUIRED)
find_package(Qt5LinguistTools "${QT_MINIMUM_VERSION}" REQUIRED)
find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)

if(USE_UTF8PROC)
    find_package(Utf8Proc REQUIRED)
endif()

include(LXQtPreventInSourceBuilds)
include(LXQtTranslateTs)
include(LXQtCompilerSettings NO_POLICY_SCOPE)
include(LXQtCreatePkgConfigFile)

if(APPLE)
    if(CMAKE_VERSION VERSION_GREATER 3.9)
        cmake_policy(SET CMP0068 NEW)
        set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
    endif()
endif()

set(QTERMWIDGET_LIBRARY_NAME qtermwidget5)


# main library

set(SRCS
    lib/BlockArray.cpp
    lib/ColorScheme.cpp
    lib/Emulation.cpp
    lib/Filter.cpp
    lib/History.cpp
    lib/HistorySearch.cpp
    lib/KeyboardTranslator.cpp
    lib/konsole_wcwidth.cpp
    lib/kprocess.cpp
    lib/kpty.cpp
    lib/kptydevice.cpp
    lib/kptyprocess.cpp
    lib/Pty.cpp
    lib/qtermwidget.cpp
    lib/Screen.cpp
    lib/ScreenWindow.cpp
    lib/SearchBar.cpp
    lib/Session.cpp
    lib/ShellCommand.cpp
    lib/TerminalCharacterDecoder.cpp
    lib/TerminalDisplay.cpp
    lib/tools.cpp
    lib/Vt102Emulation.cpp
)

# Only the Headers that need to be moc'd go here
set(HDRS
    lib/Emulation.h
    lib/Filter.h
    lib/HistorySearch.h
    lib/kprocess.h
    lib/kptydevice.h
    lib/kptyprocess.h
    lib/Pty.h
    lib/qtermwidget.h
    lib/ScreenWindow.h
    lib/SearchBar.h
    lib/Session.h
    lib/TerminalDisplay.h
    lib/Vt102Emulation.h
)

set(UI
    lib/SearchBar.ui
)

# for distribution
set(HDRS_DISTRIB
    lib/qtermwidget.h
    lib/Emulation.h
    lib/KeyboardTranslator.h
    lib/Filter.h
)

# dirs
set(KB_LAYOUT_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/kb-layouts")
message(STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}")

set(COLORSCHEMES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/color-schemes")
message(STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}" )

set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${QTERMWIDGET_LIBRARY_NAME}/translations")
message(STATUS "Translations will be installed in: ${TRANSLATIONS_DIR}")

set(QTERMWIDGET_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}")

CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)

qt5_wrap_cpp(MOCS ${HDRS})
qt5_wrap_ui(UI_SRCS ${UI})
set(PKG_CONFIG_REQ "Qt5Widgets")

lxqt_translate_ts(QTERMWIDGET_QM
    TRANSLATION_DIR "lib/translations"
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${SRCS} ${HDRS} ${UI}
    INSTALL_DIR
        ${TRANSLATIONS_DIR}
    COMPONENT
        Runtime
)

add_library(${QTERMWIDGET_LIBRARY_NAME} SHARED ${SRCS} ${MOCS} ${UI_SRCS} ${QTERMWIDGET_QM})
target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} Qt5::Widgets)
set_target_properties( ${QTERMWIDGET_LIBRARY_NAME} PROPERTIES
                       SOVERSION ${QTERMWIDGET_VERSION_MAJOR}
                       VERSION ${QTERMWIDGET_VERSION}
                     )


if(APPLE)
    target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UTMPX"
            "UTMPX_COMPAT"
    )
endif()

if(HAVE_UPDWTMPX)
    target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UPDWTMPX"
    )
endif()

if (QTERMWIDGET_USE_UTEMPTER)
    CHECK_INCLUDE_FILE(utempter.h HAVE_UTEMPTER)
    if (HAVE_UTEMPTER)
        target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME} PRIVATE
                "HAVE_UTEMPTER"
        )
        find_library(UTEMPTER_LIB NAMES utempter ulog REQUIRED)
        target_link_libraries(${QTERMWIDGET_LIBRARY_NAME} ${UTEMPTER_LIB})
    endif()
endif()

if (UTF8PROC_FOUND)
    target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UTF8PROC"
    )
    target_include_directories(${QTERMWIDGET_LIBRARY_NAME}
        INTERFACE
            ${UTF8PROC_INCLUDE_DIRS}
    )
    target_link_libraries(${QTERMWIDGET_LIBRARY_NAME}
        ${UTF8PROC_LIBRARIES}
    )
    string(APPEND PKG_CONFIG_REQ ", libutf8proc")
endif()

if(APPLE)
    set (CMAKE_SKIP_RPATH 1)
    # this is a must to load the lib correctly
    set_target_properties(${QTERMWIDGET_LIBRARY_NAME} PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

target_compile_definitions(${QTERMWIDGET_LIBRARY_NAME}
    PRIVATE
        "KB_LAYOUT_DIR=\"${KB_LAYOUT_DIR}\""
        "COLORSCHEMES_DIR=\"${COLORSCHEMES_DIR}\""
        "TRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\""
        "HAVE_POSIX_OPENPT"
        "HAVE_SYS_TIME_H"
)


generate_export_header(${QTERMWIDGET_LIBRARY_NAME}
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h"
    BASE_NAME QTERMWIDGET
)

target_include_directories(${QTERMWIDGET_LIBRARY_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/lib>"
    INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}>"
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    VERSION ${QTERMWIDGET_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(EXPORT
    "${QTERMWIDGET_LIBRARY_NAME}-targets"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(FILES
    ${HDRS_DISTRIB} "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_export.h" "${CMAKE_CURRENT_BINARY_DIR}/lib/qtermwidget_version.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)
# keyboard layouts
install(DIRECTORY
    lib/kb-layouts/
    DESTINATION "${KB_LAYOUT_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.keytab"
)
# color schemes
install(DIRECTORY
    lib/color-schemes/
    DESTINATION "${COLORSCHEMES_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.*schem*"
)

lxqt_create_pkgconfig_file(
    PACKAGE_NAME ${QTERMWIDGET_LIBRARY_NAME}
    DESCRIPTIVE_NAME ${QTERMWIDGET_LIBRARY_NAME}
    DESCRIPTION "QTermWidget library for Qt ${QTERMWIDGET_VERSION_MAJOR}.x"
    INCLUDEDIRS ${QTERMWIDGET_LIBRARY_NAME}
    LIBS ${QTERMWIDGET_LIBRARY_NAME}
    REQUIRES ${PKG_CONFIG_REQ}
    VERSION ${QTERMWIDGET_VERSION}
    INSTALL
    COMPONENT Devel
)

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}-config.cmake.in"
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/lib/qtermwidget_version.h.in"
    "${CMAKE_BINARY_DIR}/lib/qtermwidget_version.h"
    @ONLY
)

install(FILES
    "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-config.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QTERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(TARGETS ${QTERMWIDGET_LIBRARY_NAME}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    EXPORT "${QTERMWIDGET_LIBRARY_NAME}-targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER
    COMPONENT Runtime
)

export(TARGETS ${QTERMWIDGET_LIBRARY_NAME}
    FILE "${CMAKE_BINARY_DIR}/${QTERMWIDGET_LIBRARY_NAME}-targets.cmake"
    EXPORT_LINK_INTERFACE_LIBRARIES
)
# end of main library


# example application
if(BUILD_EXAMPLE)
    set(EXAMPLE_SRC examples/cpp/main.cpp)
    add_executable(example ${EXAMPLE_SRC})
    target_link_libraries(example ${QTERMWIDGET_LIBRARY_NAME})
endif()
# end of example application

# python binding
if (QTERMWIDGET_BUILD_PYTHON_BINDING)
    add_subdirectory(pyqt)
endif()
# end of python binding


CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)
ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)
