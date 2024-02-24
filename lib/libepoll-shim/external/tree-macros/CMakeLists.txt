cmake_minimum_required(VERSION 3.5)
project(tree-macros LANGUAGES C)

add_library(tree-macros INTERFACE)
target_include_directories(tree-macros
                           INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
if(APPLE)
  target_compile_definitions(tree-macros INTERFACE __uintptr_t=uintptr_t)
endif()

#

set(_namespace "${PROJECT_NAME}")

set(${PROJECT_NAME}_DIR
    "${PROJECT_BINARY_DIR}/config/subdir"
    CACHE INTERNAL "")
file(
  WRITE "${PROJECT_BINARY_DIR}/config/subdir/${PROJECT_NAME}-config.cmake"
  "
if(NOT TARGET ${_namespace}::tree-macros)
  add_library(${_namespace}::tree-macros ALIAS tree-macros)
endif()
")
