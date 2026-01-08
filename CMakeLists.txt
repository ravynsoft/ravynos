cmake_minimum_required(VERSION 4.0.0)
project(ravynOS)

# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - TOP LEVEL
# ------------------------------------------------------------------------

set(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(ROOT_BINARY_DIR ${CMAKE_BINARY_DIR})

include(./Developer/cmake/Version.cmake)
include(./Developer/cmake/Paths.cmake)
include(./Developer/cmake/Tools.cmake)

message(STATUS "Configuring ravynOS ${PROD_VERSION} ${PROD_FAMILY} for ${CpuArch}")
message(STATUS "Darwin ${DARWIN_VERSION}, Apple LLVM/TAPI ${TAPI_VERSION}")

# ------------------------------------------------------------------------
#  Top level targets
# ------------------------------------------------------------------------

add_custom_target(Darwin)
add_custom_target(Bootstrap)

# ------------------------------------------------------------------------

add_custom_target(_bsHeaders)

add_subdirectory(SystemLibrary)
add_subdirectory(Developer)
add_subdirectory(Kernel)
add_subdirectory(Libraries)

add_dependencies(Darwin Bootstrap libSystem xnu_headers)
add_dependencies(Bootstrap BootstrapTools ravynOS_sdk)
add_dependencies(_bsHeaders xnu_headers)
