# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Developer Tools
# ------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15.1)
project(ravynOS_Developer)

include(cmake/Version.cmake)
include(cmake/Paths.cmake)

add_subdirectory(xar)
add_subdirectory(cctools)
add_subdirectory(mig)
add_subdirectory(unifdef)
add_subdirectory(Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk)
add_subdirectory(Toolchains/Default.xctoolchain)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE HOST_SDKROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(EXTRA_ARGS 
        -C ${CMAKE_SOURCE_DIR}/llvm-project/tapi/cmake/caches/apple-tapi.cmake
        -DCMAKE_OSX_SYSROOT=${HOST_SDKROOT} -DLLVM_BUILD_LLVM_DYLIB=ON
        -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_CREATE_XCODE_TOOLCHAIN=ON
    )
    set(RPATH @executable_path)
else()
    set(RPATH "$ORIGIN")
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(EXTRA_ARGS -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON)
endif()


ExternalProject_Add(host_xcbuild
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/xcbuild
    CMAKE_ARGS -Wno-dev 
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
        -DCMAKE_BUILD_RPATH=${RPATH}/../lib 
        -DCMAKE_INSTALL_RPATH=${RPATH}/../lib
        -DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}
)

ExternalProject_Add(dtrace_ctf_host
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dtrace_ctf
    BINARY_DIR dtrace_ctf_host
    CMAKE_ARGS -Wno-dev
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
    INSTALL_COMMAND ""
)

file(COPY_FILE ${CMAKE_SOURCE_DIR}/mig/mig.sh ${CMAKE_BINARY_DIR}/mig/mig)
set(MIG ${CMAKE_BINARY_DIR}/mig/mig)

ExternalProject_Add(host_clang DEPENDS host_migcom
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/llvm-project/llvm
    BINARY_DIR clang
    USES_TERMINAL_BUILD
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
        -DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}/usr
        -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_LIBCXX=ON
        -DLLVM_APPEND_VC_REV=OFF -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF
        -DLLVM_INSTALL_BINUTILS_SYMLINKS=ON -DLLVM_NATIVE_TOOL_DIR=${TOOLS}
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_ZLIB=ON 
        -DLLVM_ENABLE_PROJECTS=clang$<SEMICOLON>tapi$<SEMICOLON>lld
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR} -DLLVM_BUILD_RUNTIMES=OFF
        -DCLANG_TABLEGEN_EXE=${CMAKE_CURRENT_BINARY_DIR}/clang/bin/clang-tblgen
        -DHAVE_CXX_ATOMICS_WITHOUT_LIB=1 -DHAVE_CXX_ATOMICS64_WITHOUT_LIB=1
)

set(GNU_CONFIGURE_ARGS
    --prefix=/usr --sysconfdir=/etc --bindir=/bin --sbindir=/sbin --localstatedir=/var
)
add_custom_target(host_gmake DEPENDS BootstrapTools
    COMMENT "[BUILD] GNU make"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/gmake
        && cd ${CMAKE_CURRENT_BINARY_DIR}/gmake
        && ${CMAKE_CURRENT_SOURCE_DIR}/gmake/configure ${GNU_CONFIGURE_ARGS}
        && ./build.sh
        && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/gmake/make
            ${TOOLCHAIN}/usr/bin/
)
