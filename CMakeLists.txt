cmake_minimum_required(VERSION 3.15.1)
project(ravynOS)

# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - TOP LEVEL
# ------------------------------------------------------------------------

set(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(ROOT_BINARY_DIR ${CMAKE_BINARY_DIR})

include(Developer/cmake/Version.cmake)
include(Developer/cmake/Paths.cmake)

# ------------------------------------------------------------------------
#  Top level targets
# ------------------------------------------------------------------------

add_custom_target(DarwinComplete)
add_custom_target(CoreSystemComplete)
add_custom_target(Complete)

ExternalProject_Add(EXT_Bootstrap
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer
    BINARY_DIR Bootstrap
    STEP_TARGETS build
    CMAKE_ARGS
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
    INSTALL_COMMAND ""
)

include(Developer/cmake/Tools.cmake)

ExternalProject_Add(EXT_Darwin
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel
    BINARY_DIR Kernel
    CMAKE_ARGS
        -DHOST_CC=${CMAKE_C_COMPILER}
        -DHOST_CXX=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_C_COMPILER_WORKS=ON
        -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_CXX_COMPILER_WORKS=ON
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
    DEPENDS EXT_Bootstrap-build
    INSTALL_COMMAND ""
)

add_subdirectory(SystemLibrary)
