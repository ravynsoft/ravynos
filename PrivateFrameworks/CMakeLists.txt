# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Private Frameworks
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(PrivateFrameworks LANGUAGES C CXX ASM OBJC OBJCXX)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/framework.cmake)

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_ASM_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_OBJC_COMPILER ${CC})
set(CMAKE_OBJCXX_COMPILER ${CXX})
set(CMAKE_LINK_DEPENDS_USE_LINKER OFF)

add_subdirectory(CoreUtils)