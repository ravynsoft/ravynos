# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(Libraries LANGUAGES C CXX ASM)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_ASM_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_C_USING_LINKER_APPLE ${LD})
set(CMAKE_CXX_USING_LINKER_APPLE ${LD})
set(CMAKE_LINK_DEPENDS_USE_LINKER FALSE)

add_subdirectory(libfirehose_kernel)
add_subdirectory(Libsystem)

set(LIBCXX_FLAGS
    "-target ${CpuArch}-apple-darwin -isysroot ${RAVYN_SDKROOT} \
    -isystem ${SDK_SOURCE_DIR}/usr/include/c++/v1 \
    -isystem ${RAVYN_SDKROOT}/usr/include \
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks \
    -stdlib++-isystem ${SDK_SOURCE_DIR}/usr/include/c++/v1 \
    --include=${RAVYN_SDKROOT}/usr/include/xlocale/_stdlib.h \
    --include=${RAVYN_SDKROOT}/usr/include/xlocale/_stdio.h \
    --include=${RAVYN_SDKROOT}/usr/include/xlocale/_string.h \
    --include=${RAVYN_SDKROOT}/usr/include/c++/v1/limits \
    -I${SDK_SOURCE_DIR}/usr/include \
    -I${DEVEL}/Default.xctoolchain/llvm/libcxxabi/include \
    -I${DEVEL}/Default.xctoolchain/llvm/libcxx/include \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/darwin \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/locale \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/locale/FreeBSD \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/stdtime/FreeBSD \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/private \
    -I${RAVYN_SDKROOT}/usr/include/pthread \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
    -I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
    -DFP_NAN=0 -DFP_INFINITE=1 -DFP_ZERO=2 -DFP_SUBNORMAL=3 -DFP_NORMAL=4 \
    -DINFINITY='__builtin_huge_val()' \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined"
)

ExternalProject_Add(libcxx DEPENDS BootstrapTools xnu_headers
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/runtimes
    BINARY_DIR llvm_target/libcxx
    USES_TERMINAL_BUILD
    CMAKE_ARGS -Wno-dev
        "-DCMAKE_C_FLAGS=${LIBCXX_FLAGS}"
        "-DCMAKE_CXX_FLAGS=${LIBCXX_FLAGS}"
        -DCMAKE_SYSROOT=${RAVYN_SDKROOT} -DCMAKE_SYSTEM_NAME=Darwin
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC}
        -DCMAKE_C_USING_LINKER_APPLE=${CC} -DCMAKE_CXX_USING_LINKER_APPLE=${CXX}
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release

        -DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_RUNTIMES=libcxx$<SEMICOLON>libcxxabi #$<SEMICOLON>libunwind
        #-DLLVM_ENABLE_PROJECTS=compiler-rt

        -DLIBCXX_ENABLE_EXCEPTIONS=ON -DLIBCXX_INCLUDE_TESTS=OFF
        -DLIBCXX_ENABLE_TIME_ZONE_DATABASE=OFF -DLIBCXX_INCLUDE_BENCHMARKS=OFF
        -DLIBCXX_USE_COMPILER_RT=ON -DLIBCXX_ENABLE_SHARED=OFF
        -DLIBCXX_INCLUDE_DOCS=OFF -DLIBCXX_HAS_PTHREAD_API=ON
        -DLIBCXX_HAS_ATOMIC_LIB=OFF

        #-DLIBCXXABI_USE_LLVM_UNWINDER=ON -DLIBCXXABI_ENABLE_STATIC_UNWINDER=ON
        -DLIBCXXABI_HAS_PTHREAD_API=ON -DLIBCXXABI_USE_COMPILER_RT=ON
        -DLIBCXXABI_INCLUDE_TESTS=OFF -DLIBCXXABI_ENABLE_SHARED=OFF
        -DLIBCXXABI_BAREMETAL=ON -DLIBCXXABI_SILENT_TERMINATE=ON

)

set(LIBUNWIND_FLAGS
    "-target ${CpuArch}-apple-darwin -isysroot ${RAVYN_SDKROOT} \
    -isystem ${SDK_SOURCE_DIR}/usr/include/c++/v1 \
    -isystem ${RAVYN_SDKROOT}/usr/include \
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks \
    -stdlib++-isystem ${SDK_SOURCE_DIR}/usr/include/c++/v1 \
    -I${SDK_SOURCE_DIR}/usr/include/c++/v1 -I${SDK_SOURCE_DIR}/usr/include \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/darwin \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/locale \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/locale/FreeBSD \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/libsystem_c/stdtime/FreeBSD \
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libsystem/private \
    -I${RAVYN_SDKROOT}/usr/include/pthread \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
    -I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined \
    -mmacosx-version-min=10.15"
)

ExternalProject_Add(libunwind DEPENDS BootstrapTools xnu_headers
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/runtimes
    BINARY_DIR llvm_target/libunwind
    USES_TERMINAL_BUILD
    CMAKE_ARGS -Wno-dev
        "-DCMAKE_C_FLAGS=${LIBUNWIND_FLAGS}"
        "-DCMAKE_CXX_FLAGS=${LIBUNWIND_FLAGS}"
        -DCMAKE_SYSROOT=${RAVYN_SDKROOT} -DCMAKE_SYSTEM_NAME=Darwin
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC}
        -DCMAKE_C_USING_LINKER_APPLE=${CC} -DCMAKE_CXX_USING_LINKER_APPLE=${CXX}
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release

        #-DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_RUNTIMES=libunwind

        -DLIBUNWIND_USE_COMPILER_RT=ON -DLIBUNWIND_INCLUDE_DOCS=OFF
        -DLIBUNWIND_INCLUDE_TESTS=OFF -DLIBUNWIND_IS_BAREMETAL=ON
        -DLIBUNWIND_USE_FRAME_HEADER_CACHE=OFF
        -DLIBUNWIND_REMEMBER_HEAP_ALLOC=OFF
        -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_WEAK_PTHREAD_LIB=ON
        -DLIBUNWIND_LIBCXX_PATH=${DEVEL}/Default.xctoolchain/llvm/libcxx
)

ExternalProject_Add(runtime DEPENDS BootstrapTools xnu_headers
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/llvm
    BINARY_DIR llvm_target
    USES_TERMINAL_BUILD
    CONFIGURE_ENVIRONMENT_MODIFIFCATION
        PATH=path_list_prepend:${TOOLS}
        DEVELOPER_DIR=set:${ROOT_BINARY_DIR}/Developer
        SDKROOT=set:${RAVYN_SDKROOT}
    CMAKE_ARGS -Wno-dev
        "-DCMAKE_C_FLAGS=${LIBUNWIND_FLAGS}"
        "-DCMAKE_CXX_FLAGS=${LIBUNWIND_FLAGS}"
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT}
        -DDARWIN_macosx_OVERRIDE_SDK_VERSION=10.15
        -DDARWIN_macosx_CACHED_SYSROOT=${RAVYN_SDKROOT}
        -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_C_COMPILER=${CC}
        -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_C_COMPILER_WORKS=ON
        -DCMAKE_CXX_COMPILER_WORKS=ON -DCMAKE_LINKER=${LD} 
        -DCMAKE_ASM_COMPILER=${CC} -DCMAKE_C_USING_LINKER_APPLE=${CC} -DCMAKE_CXX_USING_LINKER_APPLE=${CXX}
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release

        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DLLVM_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_PROJECTS=compiler-rt

        -DCOMPILER_RT_INCLUDE_TESTS=OFF -DCOMPILER_RT_CAN_EXECUTE_TESTS=OFF
        -DCOMPILER_RT_HAS_FLOAT16=OFF -DCOMPILER_RT_BUILD_SANITIZERS=OFF
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_BUILD_XRAY=OFF
        -DCOMPILER_RT_BUILD_PROFILE=ON -DCOMPILER_RT_BUILD_MEMPROF=OFF
        -DCOMPILER_RT_BUILD_GWP_ASAN=OFF -DCOMPILER_RT_BAREMETAL_BUILD=ON
        -DCOMPILER_RT_BUILD_SCUDO_STANDALONE_WITH_LLVM_LIBC=ON
        -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DCOMPILER_RT_INTERCEPT_LIBDISPATCH=OFF # want this ON for Darwin
        -DTEST_COMPILE_ONLY=1 -DHAVE_CXX_ATOMICS_WITHOUT_LIB=ON
        -DHAVE_CXX_ATOMICS64_WITHOUT_LIB=ON
)

add_custom_target(compiler_rt DEPENDS libcxx libunwind)

add_dependencies(libSystem BootstrapTools)
    # xnu_headers libc libplatform libpthread libmalloc #libkernel 
    #libdarwin compiler_rt libcxx
# )

# add_dependencies(Darwin libSystem libcxx)
