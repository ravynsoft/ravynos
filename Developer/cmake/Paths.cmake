
# ------------------------------------------------------------------------
#                     SET UP STUFF FOR THE BUILD
# ------------------------------------------------------------------------

set(CMAKE_MACOSX_MIN_VERSION 10.15)
set(CMAKE_INSTALL_PREFIX "/usr")
set(RUNTIME_SPEC_PATH ${ROOT_SOURCE_DIR}/Developer/xcbuild/Specifications)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(BuildArch "X86")
    set(CpuArch "x86_64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(BuildArch "AArch64")
    set(CpuArch "aarch64")
endif()

set(DEVEL ${ROOT_SOURCE_DIR}/Developer)
set(XNU_SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/xnu)
set(KEXT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/Extensions)

set(SDK_SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/ravynOS.sdk)
set(PLATFORM_SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/ravynOS.platform)
set(RAVYN_SDKROOT ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk)
set(RAVYN_SDKROOT_MACOSX ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)

set(TOOLCHAIN ${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain)
set(TOOLS ${TOOLCHAIN}/usr/bin)

set(SYSROOT_DIR ${ROOT_BINARY_DIR}/sysroot)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)

set(TARGET_TRIPLE ${CpuArch}-apple-darwin${DARWIN_MAJOR})

include(ExternalProject)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)
