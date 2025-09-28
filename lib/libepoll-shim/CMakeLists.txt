cmake_minimum_required(VERSION 3.14)
project(epoll-shim LANGUAGES C)

option(BUILD_SHARED_LIBS "build libepoll-shim as shared lib" ON)
option(ENABLE_COMPILER_WARNINGS "enable compiler warnings" OFF)

if(ENABLE_COMPILER_WARNINGS)
  add_compile_options(
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wmissing-prototypes"
    "-Werror=implicit-function-declaration"
    "-Werror=return-type"
    "-Werror=incompatible-pointer-types")
endif()

include(CTest)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

add_subdirectory(external)

include(CheckTypeSize)
list(APPEND CMAKE_EXTRA_INCLUDE_FILES "errno.h" "stdlib.h")
check_type_size(errno_t ERRNO_T BUILTIN_TYPES_ONLY LANGUAGE C)
if(NOT HAVE_ERRNO_T)
  add_definitions(-Derrno_t=int)
endif()

add_subdirectory(src)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(_namespace "${PROJECT_NAME}")

  if(BUILD_TESTING)
    file(WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
         "add_library(${_namespace}::epoll-shim ALIAS epoll-shim)\n
add_library(${_namespace}::epoll-shim-interpose ALIAS epoll-shim-interpose)\n")
    set(${PROJECT_NAME}_DIR "${PROJECT_BINARY_DIR}")
    add_subdirectory(test)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    return()
  endif()

  include(GNUInstallDirs)

  set(CMAKE_INSTALL_PKGCONFIGDIR
      "libdata/pkgconfig"
      CACHE PATH "Installation directory for pkgconfig (.pc) files")
  mark_as_advanced(CMAKE_INSTALL_PKGCONFIGDIR)
  set(CMAKE_INSTALL_CMAKEBASEDIR
      "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
      CACHE PATH "Installation directory for CMake config (.cmake) files")
  mark_as_advanced(CMAKE_INSTALL_CMAKEBASEDIR)

  foreach(_pc_filename "${PROJECT_NAME}" "${PROJECT_NAME}-interpose")
    configure_file("${PROJECT_SOURCE_DIR}/${_pc_filename}.pc.cmakein"
                   "${PROJECT_BINARY_DIR}/${_pc_filename}.pc" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${_pc_filename}.pc"
            DESTINATION "${CMAKE_INSTALL_PKGCONFIGDIR}")
  endforeach()

  set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/libepoll-shim")
  install(
    TARGETS epoll-shim epoll-shim-interpose
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY
    INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(DIRECTORY "${PROJECT_BINARY_DIR}/install-include/" TYPE INCLUDE)

  install(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE "${_namespace}::"
    DESTINATION "${CMAKE_INSTALL_CMAKEBASEDIR}")
  if(NOT BUILD_SHARED_LIBS)
    set(_deps
        "include(CMakeFindDependencyMacro)\n" #
        "set(THREADS_PREFER_PTHREAD_FLAG ON)\n" #
        "find_dependency(Threads)\n")
  endif()
  file(
    WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    ${_deps}
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")\n")
  install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
          DESTINATION "${CMAKE_INSTALL_CMAKEBASEDIR}")
endif()
