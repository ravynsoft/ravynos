
# This file is included from Kernel/CmakeLists.txt

set(ENV{DERIVED_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(Libsystem)
add_subdirectory(libplatform)
add_subdirectory(libmalloc)
# add_subdirectory(libdispatch)
add_subdirectory(libpthread)
add_subdirectory(Libc)
# add_subdirectory(CommonCrypto)
add_subdirectory(../Kernel/xnu/libsyscall libkernel)

ExternalProject_Add(compiler_rt
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/runtimes
    BINARY_DIR compiler_rt
    CONFIGURE_COMMAND
        env PATH=${TOOLS}:$ENV{PATH} ${CMAKE_COMMAND} -Wno-dev
	-S<SOURCE_DIR> -B<BINARY_DIR> -GNinja
        -DCMAKE_INSTALL_PREFIX=/tmp/foo #${RAVYN_SDKROOT}/usr
        -DCMAKE_SYSROOT=${RAVYN_SDKROOT}
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT} -DAPPLE=1 -DUNIX=1
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        #-DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_LINKER=${TOOLS}/ld64.lld
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_CROSSCOMPILING=ON
        -DCMAKE_LIPO=${LIPO} -DDARWIN_macosx_CACHED_SYSROOT=${RAVYN_SDKROOT}
        -DClang_DIR=${ROOT_BINARY_DIR}/Bootstrap/clang/lib/cmake/clang
        -DLLVM_DIR=${ROOT_BINARY_DIR}/Bootstrap/clang/lib/cmake/llvm
        "-DLLVM_ENABLE_RUNTIMES=compiler-rt$<SEMICOLON>libcxx$<SEMICOLON>libcxxabi"
        -DLLVM_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-macosx
        -DLLVM_ENABLE_LIBCXX=ON
        -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-macosx
        -DCOMPILER_RT_INCLUDE_TESTS=OFF
        -DCOMPILER_RT_STANDALONE_BUILD=ON
        -DCOMPILER_RT_BUILTINS_ENABLE_PIC=ON
        -DCOMPILER_RT_HAS_LIBC=OFF -DCOMPILER_RT_USE_BUILTINS_LIBRARY=ON
    DEPENDS libSystem #libc libplatform libmalloc libpthread
    
)

ExternalProject_Add(libcxx
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/llvm
    BINARY_DIR libcxx
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DCMAKE_INSTALL_PREFIX=/tmp/bar #${RAVYN_SDKROOT}/usr
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_SYSROOT=${RAVYN_SDKROOT} 
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
        -DCMAKE_CROSSCOMPILING=ON -DLLVM_INCLUDE_DOCS=OFF
        -DLLVM_ENABLE_RUNTIMES="libcxx$<SEMICOLON>libcxxabi"
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_DEFAULT_TARGET_TRIPLE=${TARGET_TRIPLE}
    #DEPENDS compiler_rt
    INSTALL_COMMAND /bin/echo ===== libcxx install target =====
)

add_dependencies(libSystem
    xnu_headers libc libplatform libpthread libmalloc libkernel 
    #libdarwin compiler_rt libcxx
)

add_dependencies(Darwin libSystem)
