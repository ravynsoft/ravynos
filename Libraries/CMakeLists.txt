# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(Libraries)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(Libsystem)
# add_subdirectory(libdispatch)
# add_subdirectory(CommonCrypto)

# ExternalProject_Add(compiler_rt
#     SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/llvm
#     BINARY_DIR compiler_rt
#     CONFIGURE_COMMAND
#         env PATH=${TOOLS}:$ENV{PATH} DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
#         ${CMAKE_COMMAND} -Wno-dev
# 	    -S<SOURCE_DIR> -B<BINARY_DIR> -GNinja
#         -DCMAKE_INSTALL_PREFIX=/tmp/foo #${RAVYN_SDKROOT}/usr
#         -DCMAKE_SYSROOT=${RAVYN_SDKROOT} -DAPPLE=1 -DUNIX=1
#         -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
#         -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
#         -DCMAKE_LINKER=${TOOLS}/ld64.lld
#         -DCMAKE_BUILD_TYPE=Release -DCMAKE_CROSSCOMPILING=ON
#         -DCMAKE_LIPO=${LIPO} -DDARWIN_macosx_CACHED_SYSROOT=${RAVYN_SDKROOT}
#         -DClang_DIR=${ROOT_BINARY_DIR}/Bootstrap/clang/lib/cmake/clang
#         -DLLVM_DIR=${ROOT_BINARY_DIR}/Bootstrap/clang/lib/cmake/llvm
#         -DLLVM_ENABLE_PROJECTS=compiler-rt -DLLVM_TARGETS_TO_BUILD=${BuildArch}
#         "-DLLVM_ENABLE_RUNTIMES=libcxx$<SEMICOLON>libcxxabi"
#         -DLLVM_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-macosx
#         -DLLVM_ENABLE_LIBCXX=ON -DHAVE_CXX_ATOMICS_WITHOUT_LIB=ON
#         -DHAVE_CXX_ATOMICS64_WITHOUT_LIB=ON
#         -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-macosx
#         -DCOMPILER_RT_INCLUDE_TESTS=OFF
#         -DCOMPILER_RT_STANDALONE_BUILD=ON
#         -DCOMPILER_RT_BUILTINS_ENABLE_PIC=ON
#         -DCOMPILER_RT_HAS_LIBC=OFF -DCOMPILER_RT_USE_BUILTINS_LIBRARY=ON
#         -DDARWIN_PREFER_PUBLIC_SDK=ON -DDARWIN_macosx_OVERRIDE_SDK_VERSION=10.15
#     DEPENDS libSystem #libc libplatform libmalloc libpthread
    
# )

# ExternalProject_Add(libcxx
#     SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/llvm
#     BINARY_DIR libcxx
#     CONFIGURE_COMMAND
#         env PATH=$ENV{PATH}:${TOOLS} DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
#         ${CMAKE_COMMAND} -Wno-dev
# 	    -S<SOURCE_DIR> -B<BINARY_DIR> -GNinja
#         -DCMAKE_INSTALL_PREFIX=/tmp/bar #${RAVYN_SDKROOT}/usr
#         -DCMAKE_BUILD_TYPE=Release
#         -DCMAKE_SYSROOT=${RAVYN_SDKROOT}
#         -DCMAKE_CROSSCOMPILING=ON -DLLVM_INCLUDE_DOCS=OFF
#         -DLLVM_INCLUDE_TESTS=OFF -DCMAKE_LINKER=${TOOLS}/ld64.lld
#         -DLLVM_TARGETS_TO_BUILD=${BuildArch}
#         -DLLVM_ENABLE_RUNTIMES=libcxx
#         -DLLVM_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-macosx
#     BUILD_COMMAND ninja -C <BINARY_DIR>
#     DEPENDS libc
#     INSTALL_COMMAND /bin/echo ===== libcxx install target =====
# )

add_dependencies(libSystem BootstrapTools)
    # xnu_headers libc libplatform libpthread libmalloc #libkernel 
    #libdarwin compiler_rt libcxx
# )

# add_dependencies(Darwin libSystem libcxx)
