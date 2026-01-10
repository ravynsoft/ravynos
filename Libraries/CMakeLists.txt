# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(Libraries)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_C_USING_LINKER_APPLE ${LD})
set(CMAKE_CXX_USING_LINKER_APPLE ${LD})
set(CMAKE_LINK_DEPENDS_USE_LINKER FALSE)

add_subdirectory(Libsystem)
# add_subdirectory(libdispatch)
# add_subdirectory(CommonCrypto)

ExternalProject_Add(compiler_rt DEPENDS BootstrapTools xnu_headers
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/llvm
    BINARY_DIR llvm_target
    USES_TERMINAL_BUILD
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev -DCMAKE_SYSTEM_NAME=Darwin
        "-DCMAKE_C_FLAGS=--target x86_64-apple-darwin --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -isystem ${RAVYN_SDKROOT}/usr/include -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks"
        "-DCMAKE_CXX_FLAGS=--target x86_64-apple-darwin --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -isystem ${RAVYN_SDKROOT}/usr/include -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks"
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT}
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        #-DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC}
        -DCMAKE_C_USING_LINKER_APPLE=${CC} -DCMAKE_CXX_USING_LINKER_APPLE=${CXX}
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release

        -DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_PROJECTS=clang$<SEMICOLON>compiler-rt
        -DLLVM_ENABLE_RUNTIMES=libcxx$<SEMICOLON>libcxxabi$<SEMICOLON>libunwind
        -DLIBCXX_ENABLE_EXCEPTIONS=ON -DCOMPILER_RT_INCLUDE_TESTS=OFF
        -DCOMPILER_RT_HAS_FLOAT16=OFF
)


add_dependencies(libSystem BootstrapTools)
    # xnu_headers libc libplatform libpthread libmalloc #libkernel 
    #libdarwin compiler_rt libcxx
# )

# add_dependencies(Darwin libSystem libcxx)
