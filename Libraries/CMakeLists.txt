
# This file is included from Kernel/CmakeLists.txt

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(Libsystem)
add_subdirectory(libplatform)
add_subdirectory(libmalloc)
# add_subdirectory(libdispatch)
add_subdirectory(libpthread)
add_subdirectory(Libc)
# add_subdirectory(CommonCrypto)

ExternalProject_Add(compiler_rt
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/runtimes
    BINARY_DIR compiler_rt
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT}
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_CROSSCOMPILING=ON
        -DLLVM_ENABLE_RUNTIMES=compiler-rt
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
        -DCOMPILER_RT_INCLUDE_TESTS=OFF
    DEPENDS libc libplatform libmalloc libpthread
)

# list(APPEND l_cflags
#     -nostdinc -nostdlib
#     -Wno-user-defined-literals;
#     -Wno-expansion-to-defined;
#     -Wno-nullability-completeness;
#     -I${ROOT_SOURCE_DIR}/Libraries/libmalloc/include;
#     -I${ROOT_SOURCE_DIR}/Libraries/libpthread;
#     -I${ROOT_SOURCE_DIR}/Libraries/libpthread/pthread;
#     -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers;
#     -I${ROOT_SOURCE_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk/copyfile
#     -I<SOURCE_DIR>/../libcxxabi/include
#     -I${ROOT_SOURCE_DIR}/Libraries/dyld/include
#     -I${RAVYN_SDKROOT}/usr/include
#     -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
#     -I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
#     -I${ROOT_SOURCE_DIR}/Developer/llvm-project/libcxx/include
#     -DFP_NORMAL=0 -DFP_SUBNORMAL=1 -DFP_ZERO=2 -DFP_INFINITE=3 -DFP_NAN=4
#     -DFLT_MANT_DIG=__FLT_MANT_DIG__ -DDBL_MANT_DIG=__DBL_MANT_DIG__
#     -DLDBL_MANT_DIG=__LDBL_MANT_DIG__ -DFLT_MAX_EXP=__FLT_MAX_EXP__
#     -DFLT_MIN_EXP=__FLT_MIN_EXP__ -DDBL_MIN_EXP=__DBL_MIN_EXP__
#     -DDBL_MAX_EXP=__DBL_MAX_EXP__ "-DINFINITY=__HUGE_FLT__"
# )
# list(JOIN l_cflags $<SEMICOLON> ADD_CFLAGS)

ExternalProject_Add(libcxx
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/runtimes
    BINARY_DIR libcxx
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT} -DCMAKE_SYSTEM_NAME=Darwin
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_CROSSCOMPILING=ON -DLLVM_INCLUDE_DOCS=OFF
        -DLLVM_ENABLE_RUNTIMES="libcxx$<SEMICOLON>libcxxabi"
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
    DEPENDS compiler_rt
    INSTALL_COMMAND ""
)

add_dependencies(libSystem
    xnu_headers libc libplatform libpthread libmalloc #libdarwin
    compiler_rt libcxx)

add_dependencies(Darwin libSystem)
