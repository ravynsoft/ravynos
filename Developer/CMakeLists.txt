add_subdirectory(xar)
add_subdirectory(cctools)
add_subdirectory(mig)
add_subdirectory(unifdef)
add_subdirectory(Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk)
add_subdirectory(Toolchains/Default.xctoolchain)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    set(RPATH @executable_path)
    set(HOST_CC ${CC})
    set(HOST_CXX ${CXX})
else()
    set(RPATH "$ORIGIN")
    set(HOST_CC /usr/bin/clang)
    set(HOST_CXX /usr/bin/clang++)
endif()
ExternalProject_Add(host_xcbuild
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Developer/xcbuild
    CMAKE_ARGS -Wno-dev 
        -DCMAKE_BUILD_RPATH=${RPATH}/../lib 
        -DCMAKE_INSTALL_RPATH=${RPATH}/../lib
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain
)

ExternalProject_Add(dtrace_ctf_host
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Developer/dtrace_ctf
    BINARY_DIR ${CMAKE_BINARY_DIR}/Developer/dtrace_ctf_host
    INSTALL_COMMAND ""
)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(BuildArch "X86")
    set(CpuArch "x86_64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(BuildArch "AArch64")
    set(CpuArch "aarch64")
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE HOST_SDKROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message("Darwin build: found sdkroot=${HOST_SDKROOT}")
    set(EXTRA_ARGS 
        -C ${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/tapi/cmake/caches/apple-tapi.cmake
        -DCMAKE_OSX_SYSROOT=${HOST_SDKROOT} -DLLVM_BUILD_LLVM_DYLIB=ON
        -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_CREATE_XCODE_TOOLCHAIN=ON
    )
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(EXTRA_ARGS -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON)
endif()

file(COPY_FILE ${CMAKE_SOURCE_DIR}/Developer/mig/mig.sh ${CMAKE_BINARY_DIR}/Developer/mig/mig)
set(MIG ${CMAKE_BINARY_DIR}/Developer/mig/mig)
ExternalProject_Add(host_clang DEPENDS host_migcom
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Developer/llvm-project/llvm
    BINARY_DIR ${CMAKE_BINARY_DIR}/Developer/clang
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev 
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr
        -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_TARGETS_TO_BUILD=${BuildArch} -DLLVM_ENABLE_LTO=OFF
        -DLLVM_DEFAULT_TARGET_TRIPLE=${CpuArch}-ravynos-darwin-macho
        -DLLVM_BUILD_DOCS=OFF -DLLVM_INSTALL_BINUTILS_SYMLINKS=ON
        -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_ZLIB=ON -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_ENABLE_PROJECTS=clang$<SEMICOLON>tapi$<SEMICOLON>lld
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR} -DLLVM_BUILD_RUNTIMES=OFF
        -DCLANG_TABLEGEN_EXE=${CMAKE_CURRENT_BINARY_DIR}/clang/bin/clang-tblgen
        -DHAVE_CXX_ATOMICS_WITHOUT_LIB=1 -DHAVE_CXX_ATOMICS64_WITHOUT_LIB=1
)

# ExternalProject_Add(compilerRT
#     SOURCE_DIR ${CMAKE_SOURCE_DIR}/Developer/llvm-project/compiler-rt
#     BINARY_DIR ${CMAKE_BINARY_DIR}/Developer/compiler-rt
#     CMAKE_ARGS
#         ${EXTRA_ARGS} -Wno-dev
#         -DCMAKE_INSTALL_PREFIX=<BINARY_DIR> #${CMAKE_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr
#         -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=${BuildArch}
#         -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_ZLIB=ON
#         -DLLVM_DIR=<BINARY_DIR>/../clang
#         -DCOMPILER_RT_BAREMETAL_BUILD=ON -DCOMPILER_RT_BUILD_LIBFUZZER=OFF
#         -DCOMPILER_RT_BUILD_PROFILE=OFF -DCOMPILER_RT_BUILD_SANITIZERS=OFF
#         -DCOMPILER_RT_BUILD_XRAY=OFF -DCOMPILER_RT_INCLUDE_TESTS=OFF
#         -DCMAKE_C_COMPILER=<BINARY_DIR>/../clang/bin/clang
#         -DCMAKE_CXX_COMPILER=<BINARY_DIR>/../clang/bin/clang++
#         -DCMAKE_AR=<BINARY_DIR>/../clang/bin/llvm-ar
#         -DCMAKE_NM=<BINARY_DIR>/../clang/bin/llvm-nm
#         -DCMAKE_RANLIB=<BINARY_DIR>/../clang/bin/llvm-ranlib
#         -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CpuArch}-ravynos-darwin-macho
#         -DCMAKE_SYSROOT=${ROOT_BINARY_DIR}/sysroot
#         -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
#     DEPENDS ravynOS_sdk host_clang
# )

set(GNU_CONFIGURE_ARGS
    --prefix=/usr --sysconfdir=/etc --bindir=/bin --sbindir=/sbin --localstatedir=/var
)
add_custom_target(host_gmake ALL DEPENDS BootstrapTools
    USES_TERMINAL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/gmake
        && cd ${CMAKE_CURRENT_BINARY_DIR}/gmake
        && CC=${HOST_CC} CXX=${HOST_CXX} 
            ${CMAKE_CURRENT_SOURCE_DIR}/gmake/configure ${GNU_CONFIGURE_ARGS}
        && ./build.sh
        && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/gmake/make
            ${TOOLCHAIN}/usr/bin/
)
