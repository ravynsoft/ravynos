
# ------------------------------------------------------------------------
#                     SET UP STUFF FOR THE BUILD
# ------------------------------------------------------------------------

set(CMAKE_MACOSX_MIN_VERSION 12.0)

set(SYSROOT_DIR ${ROOT_BINARY_DIR}/sysroot)
set(RAVYN_SDKROOT ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk)
set(TOOLCHAIN ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain)

set(RUNTIME_SPEC_PATH ${ROOT_SOURCE_DIR}/Developer/xcbuild/Specifications)
set(CMAKE_INSTALL_PREFIX "/usr")

if("${GMAKE}" STREQUAL "")
    if(CMAKE_HOST_BSD)
        set(GMAKE gmake)
        set(BMAKE make)
    else()
        set(GMAKE make)
        set(BMAKE bmake)
    endif()
endif()

include(ExternalProject)
include(${ROOT_SOURCE_DIR}/Developer/cmake/asm.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/bundle_resources.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/circular.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/crosscompile.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/get_target_dependencies.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/install_helpers.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/kext.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/mig.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/suppress_warnings.cmake)
