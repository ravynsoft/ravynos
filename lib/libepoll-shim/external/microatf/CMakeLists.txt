cmake_minimum_required(VERSION 3.5)
project(microatf LANGUAGES C)

#

include(CTest)

#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
add_definitions(-D_GNU_SOURCE=1)

#

add_subdirectory(src)

#

set(_namespace "${PROJECT_NAME}")

#

set(${PROJECT_NAME}_DIR
    "${PROJECT_BINARY_DIR}/config/subdir"
    CACHE INTERNAL "")
file(
  WRITE "${PROJECT_BINARY_DIR}/config/subdir/${PROJECT_NAME}-config.cmake"
  "
list(APPEND CMAKE_MODULE_PATH \"${PROJECT_SOURCE_DIR}/cmake\")
if(NOT TARGET ${_namespace}::microatf-c)
  add_library(${_namespace}::microatf-c ALIAS microatf-c)
endif()
if(NOT TARGET ${_namespace}::microatf-translate-signal)
  add_executable(${_namespace}::microatf-translate-signal ALIAS microatf-translate-signal)
endif()
")

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()

  if(CMAKE_VERSION VERSION_LESS "3.14.0")
    return()
  endif()

  include(GNUInstallDirs)

  set(CMAKE_INSTALL_CMAKEBASEDIR
      "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
      CACHE PATH "Installation directory for CMake config (.cmake) files")
  mark_as_advanced(CMAKE_INSTALL_CMAKEBASEDIR)

  install(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE "${_namespace}::"
    DESTINATION "${CMAKE_INSTALL_CMAKEBASEDIR}")
  file(
    WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "
list(APPEND CMAKE_MODULE_PATH \"\${CMAKE_CURRENT_LIST_DIR}\")
include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")
")
  install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
          DESTINATION "${CMAKE_INSTALL_CMAKEBASEDIR}")

  file(
    WRITE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "
list(APPEND CMAKE_MODULE_PATH \"${PROJECT_SOURCE_DIR}/cmake\")
include(\"${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake\")
")
  export(
    EXPORT ${PROJECT_NAME}-targets
    NAMESPACE "${_namespace}::"
    FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake")

  #

  set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/microatf")
  install(
    TARGETS microatf-c
    EXPORT ${PROJECT_NAME}-targets
    INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(
    TARGETS microatf-translate-signal
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")
  install(FILES "${PROJECT_SOURCE_DIR}/src/atf-c.h" TYPE INCLUDE)
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/cmake/"
          DESTINATION "${CMAKE_INSTALL_CMAKEBASEDIR}")
endif()
