cmake_minimum_required(VERSION 3.15.1)
project(dtrace_ctf_host)

cmake_policy(SET CMP0114 NEW)

set(CMAKE_INSTALL_PREFIX "")
set(CMAKE_MACOSX_MIN_VERSION 11.0)

include(ExternalProject)
include(../cmake/asm.cmake)
include(../cmake/bundle_resources.cmake)
include(../cmake/circular.cmake)
include(../cmake/crosscompile.cmake)
include(../cmake/get_target_dependencies.cmake)
include(../cmake/install_helpers.cmake)
include(../cmake/kext.cmake)
include(../cmake/mig.cmake)
include(../cmake/suppress_warnings.cmake)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    add_compile_definitions(
        __APPLE__ __DARWIN_UNIX03 __DARWIN_ONLY_UNIX_CONFORMANCE
        __private_extern__=extern
        __unused=
    )
    add_compile_options(-w
    )
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}../llvm-project/llvm/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../cctools/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../cctools/include/foreign
        ${CMAKE_SOURCE_DIR}/../../Kernel/xnu/EXTERNAL_HEADERS
        ${CMAKE_SOURCE_DIR}/../../Kernel/xnu/osfmk
    )
endif()

add_subdirectory(libctf)
add_subdirectory(libdwarf)
add_subdirectory(libelf)
add_subdirectory(tools)
