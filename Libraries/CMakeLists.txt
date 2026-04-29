# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0.0)
project(Libraries LANGUAGES C CXX ASM OBJC OBJCXX)

include(${ROOT_SOURCE_DIR}/Developer/cmake/Version.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Paths.cmake)
include(${ROOT_SOURCE_DIR}/Developer/cmake/Tools.cmake)

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_ASM_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_OBJC_COMPILER ${CC})
set(CMAKE_OBJCXX_COMPILER ${CXX})
set(CMAKE_LINK_DEPENDS_USE_LINKER OFF)

add_subdirectory(libfirehose_kernel)
add_dependencies(xnu libfirehose_kernel)
add_subdirectory(CrashReporterClient)
add_subdirectory(dyld)
add_subdirectory(Libsystem)
add_subdirectory(objc4)
add_subdirectory(MiscLibs)

# ------------------------------------------------------------------------
#                  LLVM BUILTINS AND RUNTIME
# ------------------------------------------------------------------------

set(RUNTIME_FLAGS
    "--sysroot=${RAVYN_SDKROOT_MACOSX} \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
    -I${RAVYN_SDKROOT}/usr/local/include/kernel \
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined \
    -Wno-unused-command-line-argument \
    -mmacosx-version-min=10.13"
)
ExternalProject_Add(runtime DEPENDS xnu_headers
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/compiler-rt
    BINARY_DIR llvm_target/compiler-rt
    USES_TERMINAL_BUILD
    CONFIGURE_ENVIRONMENT_MODIFIFCATION
        PATH=path_list_prepend:${TOOLS}
        DEVELOPER_DIR=set:${ROOT_BINARY_DIR}/Developer
        SDKROOT=set:${RAVYN_SDKROOT_MACOSX}
    CMAKE_ARGS -Wno-dev
        -C${DEVEL}/Default.xctoolchain/llvm/compiler-rt/cmake/caches/Apple.cmake
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR} -DDEVEL=${DEVEL}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
        -DCMAKE_C_FLAGS=${RUNTIME_FLAGS} -DCMAKE_CXX_FLAGS=${RUNTIME_FLAGS}
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_ASM_COMPILER_TARGET=${CpuArch}-apple-darwin
        -DCMAKE_C_COMPILER_TARGET=${CpuArch}-apple-darwin
        -DCMAKE_CXX_COMPILER_TARGET=${CpuArch}-apple-darwin
	"-DCMAKE_SHARED_LINKER_FLAGS=-L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system"
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT_MACOSX}
        -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON -DCOMPILER_RT_OS_DIR=ravynos
        -DCOMPILER_RT_BUILD_BUILTINS=ON -DCOMPILER_RT_BUILD_SANITIZERS=OFF
        -DCOMPILER_RT_BUILD_PROFILE=ON -DCOMPILER_RT_BUILD_XRAY=OFF
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_BUILD_MEMPROF=OFF
        -DCOMPILER_RT_BUILD_ORC=OFF -DCOMPILER_RT_EXTERNALIZE_DEBUGINFO=OFF
        -DCOMPILER_RT_HAS_ATOMIC_KEYWORD=ON -DCOMPILER_RT_EXCLUDE_ATOMIC_BUILTIN=OFF
)

# ------------------------------------------------------------------------
#                           LIBUNWIND
# ------------------------------------------------------------------------
set(LIBUNWIND_FLAGS
    "-target ${CpuArch}-apple-darwin -isysroot${RAVYN_SDKROOT} \
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
    -I${RAVYN_SDKROOT}/usr/include/c++/v1 \
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined \
    -mmacosx-version-min=10.15"
)

ExternalProject_Add(libunwind DEPENDS xnu_headers runtime
    SOURCE_DIR ${DEVEL}/Default.xctoolchain/llvm/runtimes
    BINARY_DIR llvm_target/libunwind
    USES_TERMINAL_BUILD
    CMAKE_ARGS -Wno-dev
        "-DCMAKE_C_FLAGS=${LIBUNWIND_FLAGS}"
        "-DCMAKE_CXX_FLAGS=${LIBUNWIND_FLAGS}"
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT} -DCMAKE_SYSTEM_NAME=Darwin
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC}
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release
        "-DCMAKE_SHARED_LINKER_FLAGS=-L${RAVYN_SDKROOT}/usr/lib/system"

        -DLLVM_ENABLE_LTO=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_RUNTIMES=libunwind

        -DLIBUNWIND_USE_COMPILER_RT=ON -DLIBUNWIND_INCLUDE_DOCS=OFF
        -DLIBUNWIND_INCLUDE_TESTS=OFF -DLIBUNWIND_USE_FRAME_HEADER_CACHE=OFF
        -DLIBUNWIND_REMEMBER_HEAP_ALLOC=OFF
        -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_WEAK_PTHREAD_LIB=ON
        -DLIBUNWIND_LIBCXX_PATH=${DEVEL}/Default.xctoolchain/llvm/libcxx
)

# ------------------------------------------------------------------------
#                             LIBC++
# ------------------------------------------------------------------------

set(LIBCXX_FLAGS
    "-target ${CpuArch}-apple-darwin -nostdinc -nobuiltininc \
    -isystem${RAVYN_SDKROOT}/usr/include \
    -DCRT_HAS_128BIT \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined"
)

ExternalProject_Add(libcxx DEPENDS xnu_headers libunwind
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
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release
        "-DCMAKE_SHARED_LINKER_FLAGS=-nodefaultlibs \
        -Wl,-install_name,/usr/lib/libc++.dylib \
        -L${RAVYN_SDKROOT}/usr/lib \
        -L${ROOT_BINARY_DIR}/Libraries/llvm_target/libunwind/lib \
	-lunwind -lSystem \
	-L${RAVYN_SDKROOT}/usr/lib/ravynos -lclang_rt.builtins-${CpuArch}"
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=OFF

        -DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_RUNTIMES=libcxx$<SEMICOLON>libcxxabi

        -DLIBCXX_ENABLE_EXCEPTIONS=ON -DLIBCXX_INCLUDE_TESTS=OFF
        -DLIBCXX_ENABLE_TIME_ZONE_DATABASE=OFF -DLIBCXX_INCLUDE_BENCHMARKS=OFF
	-DLIBCXX_USE_COMPILER_RT=ON -DLIBCXX_ENABLE_SHARED=OFF
        -DLIBCXX_INCLUDE_DOCS=OFF -DLIBCXX_HAS_PTHREAD_API=ON
        -DLIBCXX_HAS_ATOMIC_LIB=OFF

	-DLIBCXXABI_USE_LLVM_UNWINDER=OFF
	-DLIBCXXABI_HAS_PTHREAD_API=ON -DLIBCXXABI_USE_COMPILER_RT=OFF
	-DLIBCXXABI_INCLUDE_TESTS=OFF -DLIBCXXABI_ENABLE_SHARED=OFF
        -DLIBCXXABI_SILENT_TERMINATE=ON
)

add_custom_target(compiler_rt DEPENDS libcxx
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system &&
        ${CC} -o ${RAVYN_SDKROOT}/usr/lib/system/libcompiler_rt.dylib
	-mmacosx-version-min=${MACOS_VERSION_MIN} -shared
        -Wl,-install_name,/usr/lib/system/libcompiler_rt.dylib
	-Wl,-current_version,${LLVM_VERSION} -umbrella System
	-Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/llvm_target/compiler-rt/lib/ravynos/libclang_rt.builtins-${CpuArch}.a
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib -lsystem_platform
	-lsystem_c -lsystem_malloc -lsystem_m -ldispatch -ldyld -lsystem_kernel 
	-lsystem_pthread -lunwind)

add_custom_target(libcxx_dylib DEPENDS libcxx compiler_rt
    # Evil Hack(tm) to create a combined libc++ and libc++abi as our dylib
    COMMAND ${CC} -o ${RAVYN_SDKROOT}/usr/lib/libc++.dylib
        -mmacos-version-min=${MACOS_VERSION_MIN} -shared 
	-Wl,-install_name,/usr/lib/libc++.dylib
	-Wl,-current_version,${LLVM_VERSION}
	-Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/llvm_target/libcxx/lib/libc++.a
	-Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/llvm_target/libcxx/lib/libc++abi.a
	${CMAKE_CURRENT_BINARY_DIR}/llvm_target/compiler-rt/lib/ravynos/libclang_rt.builtins-${CpuArch}.a
	-L${RAVYN_SDKROOT}/usr/lib -lSystem
)
add_dependencies(compiler_rt libcxx libunwind runtime)
add_dependencies(xnu compiler_rt)
add_dependencies(libdyld libsystem_malloc libsystem_c libsystem_pthread)
add_dependencies(libSystem
    xnu_headers libsystem_c libsystem_platform libsystem_pthread
    libsystem_malloc libdyld libkernel libsystem_darwin
    compiler_rt
)

add_dependencies(Darwin libSystem libcxx)
