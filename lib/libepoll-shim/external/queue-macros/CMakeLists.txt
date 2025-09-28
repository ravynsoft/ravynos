cmake_minimum_required(VERSION 3.5)
project(queue-macros LANGUAGES C)

add_library(queue-macros INTERFACE)
target_include_directories(queue-macros
                           INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

#

set(_namespace "${PROJECT_NAME}")

set(${PROJECT_NAME}_DIR
    "${PROJECT_BINARY_DIR}/config/subdir"
    CACHE INTERNAL "")
file(
  WRITE "${PROJECT_BINARY_DIR}/config/subdir/${PROJECT_NAME}-config.cmake"
  "
if(NOT TARGET ${_namespace}::queue-macros)
  add_library(${_namespace}::queue-macros ALIAS queue-macros)
endif()
")
