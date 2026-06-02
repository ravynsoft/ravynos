# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Darwin Base System
# ------------------------------------------------------------------------

DERIVED_DATA_DIR = ${.OBJDIR}
MK_WERROR = no
CXXLIB = ${RAVYN_SDKROOT}/usr/lib/libc++.dylib
RTLIB = ${RAVYN_SDKROOT}/usr/lib/system/libcompiler_rt.dylib

SUBDIR = libfirehose_kernel CrashReporterClient MiscLibs objc4 dyld Libsystem

all: libcxx_dylib compiler_rt

# ------------------------------------------------------------------------
#                  LLVM BUILTINS AND RUNTIME
# ------------------------------------------------------------------------

RUNTIME_FLAGS = "--sysroot=${RAVYN_SDKROOT} -B${TOOLCHAIN}/usr/bin \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
	-Wno-incompatible-sysroot -Wno-user-defined-literals \
	-Wno-nullability-completeness -Wno-expansion-to-defined \
	-Wno-unused-command-line-argument \
	-mmacosx-version-min=10.13 -fuse-ld=${LD}"
	

runtime:
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer \
	PATH=${TOOLCHAIN}/usr/bin:${PATH} \
	SDKROOT=${RAVYN_SDKROOT} CFLAGS=${RUNTIME_FLAGS} \
	cmake -S ${DEVEL}/Default.xctoolchain/llvm/compiler-rt \
	-B ${.OBJDIR}/llvm_target/compiler-rt -Wno-dev \
	-C${DEVEL}/Default.xctoolchain/llvm/compiler-rt/cmake/caches/Apple.cmake \
	-DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} \
	-DCMAKE_ASM_COMPILER=${CC} -DCMAKE_C_COMPILER_WORKS=ON \
	-DCMAKE_CXX_COMPILER_WORKS=ON -DCMAKE_SYSROOT=${RAVYN_SDKROOT} \
	-DCMAKE_LINKER=${LD} -DCMAKE_SYSTEM_NAME=Darwin \
	-DCMAKE_ASM_COMPILER_TARGET=${CpuArch}-apple-darwin \
	-DCMAKE_C_COMPILER_TARGET=${CpuArch}-apple-darwin \
	-DCMAKE_CXX_COMPILER_TARGET=${CpuArch}-apple-darwin \
	-DCMAKE_C_CFLAGS_INIT=${RUNTIME_FLAGS} -DCMAKE_LIPO=${LIPO} \
	"-DCMAKE_SHARED_LINKER_FLAGS=-L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system" \
	-DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr \
	-DCOMPILER_RT_ENABLE_WERROR=OFF -DCOMPILER_RT_HAS_G_FLAG=ON \
	-DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=${BuildArch} \
	-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON -DCOMPILER_RT_OS_DIR=ravynos \
	-DCOMPILER_RT_BUILD_BUILTINS=ON -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
	-DCOMPILER_RT_BUILD_PROFILE=ON -DCOMPILER_RT_BUILD_XRAY=OFF \
	-DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_BUILD_MEMPROF=OFF \
	-DCOMPILER_RT_BUILD_ORC=OFF -DCOMPILER_RT_EXTERNALIZE_DEBUGINFO=OFF \
	-DCOMPILER_RT_HAS_ATOMIC_KEYWORD=ON -DCOMPILER_RT_EXCLUDE_ATOMIC_BUILTIN=OFF 
	env MAKEFLAGS="" .MAKEFLAGS="" \
		${GMAKE} -C${.OBJDIR}/llvm_target/compiler-rt -j${.MAKE.JOBS}
	cp -f ${.OBJDIR}/llvm_target/compiler-rt/lib/ravynos/libclang_rt.osx.a \
	  ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${.OBJDIR}/llvm_target/compiler-rt/lib/ravynos/libclang_rt.cc_kext.a \
	  ${RAVYN_SDKROOT}/usr/local/lib/kernel/libcc_kext.a


# ------------------------------------------------------------------------
#                           LIBUNWIND
# ------------------------------------------------------------------------
LIBUNWIND_FLAGS = -target ${CpuArch}-apple-darwin -isysroot${RAVYN_SDKROOT} \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd -fuse-ld=${LD} \
	-I${RAVYN_SDKROOT}/usr/include/c++/v1 \
	-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
	-Wno-incompatible-sysroot -Wno-user-defined-literals \
	-Wno-nullability-completeness -Wno-expansion-to-defined \
	-mmacosx-version-min=${MACOS_VERSION_MIN}
Clang_DIR = ${ROOT_BINARY_DIR}${DEVEL}/Default.xctoolchain/llvm_host/lib/cmake/clang
LLVM_DIR = ${ROOT_BINARY_DIR}${DEVEL}/Default.xctoolchain/llvm_host/lib/cmake/llvm

libunwind: runtime
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer \
	PATH=${TOOLCHAIN}/usr/bin:${PATH} \
	SDKROOT=${RAVYN_SDKROOT} \
	cmake -S ${DEVEL}/Default.xctoolchain/llvm/runtimes \
	-B ${.OBJDIR}/llvm_target/libunwind -Wno-dev \
        -DCMAKE_C_FLAGS="${LIBUNWIND_FLAGS}" \
	-DCMAKE_CXX_FLAGS="${LIBUNWIND_FLAGS}" \
	-DCMAKE_SYSTEM_NAME=Generic -DCMAKE_ASM_FLAGS="${LIBUNWIND_FLAGS}" \
	-DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} \
	-DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON \
	-DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC} \
	-DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release \
	"-DCMAKE_SHARED_LINKER_FLAGS=-L${RAVYN_SDKROOT}/usr/lib/system" \
	-DLLVM_ENABLE_LTO=OFF -DLLVM_INCLUDE_TESTS=OFF \
	-DLLVM_ENABLE_WARNINGS=OFF \
	-DLLVM_ENABLE_RUNTIMES=libunwind \
	-DLIBUNWIND_USE_COMPILER_RT=ON -DLIBUNWIND_INCLUDE_DOCS=OFF \
	-DLIBUNWIND_INCLUDE_TESTS=OFF -DLIBUNWIND_USE_FRAME_HEADER_CACHE=OFF \
	-DLIBUNWIND_REMEMBER_HEAP_ALLOC=OFF \
	-DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_WEAK_PTHREAD_LIB=ON \
	-DLIBUNWIND_LIBCXX_PATH=${DEVEL}/Default.xctoolchain/llvm/libcxx
	env MAKEFLAGS="" .MAKEFLAGS="" \
		${GMAKE} -C${.OBJDIR}/llvm_target/libunwind -j${.MAKE.JOBS}
	cp -f ${.OBJDIR}/llvm_target/libunwind/lib/libunwind.a \
		${RAVYN_SDKROOT}/usr/lib/
	cp -f ${DEVEL}/Default.xctoolchain/llvm/libunwind/include/unwind.h \
		${DEVEL}/Default.xctoolchain/llvm/libunwind/include/libunwind.h \
		${DEVEL}/Default.xctoolchain/llvm/libunwind/include/unwind_itanium.h \
		${DEVEL}/Default.xctoolchain/llvm/libunwind/include/__libunwind_config.h \
		${RAVYN_SDKROOT}/usr/include/


# ------------------------------------------------------------------------
#                             LIBC++
# ------------------------------------------------------------------------

LIBCXX_FLAGS = --target=${CpuArch}-apple-darwin \
    -Wno-macro-redefined -isystem${RAVYN_SDKROOT}/usr/include/c++/v1 \
    -isystem${TOOLCHAIN}/usr/lib/clang/17/include -std=c++20 \
    -I${RAVYN_SDKROOT}/usr/local/include -I${RAVYN_SDKROOT}/usr/include \
    -DCRT_HAS_128BIT -fuse-ld=${LD} -Wno-unused-command-line-argument \
    -Wno-incompatible-sysroot -Wno-user-defined-literals \
    -Wno-nullability-completeness -Wno-expansion-to-defined

libcxx: libunwind
	PATH=${TOOLCHAIN}/usr/bin:${PATH} \
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer \
	SDKROOT=${RAVYN_SDKROOT} \
	cmake -S ${DEVEL}/Default.xctoolchain/llvm/runtimes \
	-B ${.OBJDIR}/llvm_target/libcxx -Wno-dev \
        -DCMAKE_C_FLAGS="${LIBCXX_FLAGS}" \
        -DCMAKE_CXX_FLAGS="${LIBCXX_FLAGS}" \
        -DCMAKE_OSX_SYSROOT=${RAVYN_SDKROOT} -DCMAKE_SYSTEM_NAME=Darwin \
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} \
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON \
        -DCMAKE_LINKER=${LD} -DCMAKE_ASM_COMPILER=${CC} \
        -DCMAKE_LINK_DEPENDS_USE_LINKER=OFF \
        -DCMAKE_INSTALL_PREFIX=${RAVYN_SDKROOT}/usr -DCMAKE_BUILD_TYPE=Release \
        "-DCMAKE_SHARED_LINKER_FLAGS=-nodefaultlibs -Wl,-install_name,/usr/lib/libc++.dylib -L${RAVYN_SDKROOT}/usr/lib -L${ROOT_BINARY_DIR}/Libraries/llvm_target/libunwind/lib -lunwind -lSystem" \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=OFF \
        -DLLVM_ENABLE_LIBCXX=ON \
        -DLLVM_ENABLE_LTO=OFF -DLLVM_BUILD_DOCS=OFF -DLLVM_INCLUDE_TESTS=OFF \
        -DLLVM_ENABLE_WARNINGS=OFF \
        -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
        -DLIBCXX_ENABLE_EXCEPTIONS=ON -DLIBCXX_INCLUDE_TESTS=OFF \
        -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
	-DLIBCXX_USE_COMPILER_RT=ON -DLIBCXX_ENABLE_SHARED=OFF \
        -DLIBCXX_INCLUDE_DOCS=OFF -DLIBCXX_HAS_PTHREAD_API=ON \
        -DLIBCXX_HAS_ATOMIC_LIB=OFF \
	-DLIBCXXABI_USE_LLVM_UNWINDER=OFF \
	-DLIBCXXABI_HAS_PTHREAD_API=ON -DLIBCXXABI_USE_COMPILER_RT=OFF \
	-DLIBCXXABI_INCLUDE_TESTS=OFF -DLIBCXXABI_ENABLE_SHARED=OFF \
        -DLIBCXXABI_SILENT_TERMINATE=ON \
	&& \
	env MAKEFLAGS="" .MAKEFLAGS="" MACOSX_DEPLOYMENT_TARGET="" \
		${GMAKE} -C${.OBJDIR}/llvm_target/libcxx -j${.MAKE.JOBS}
	cp -fv ${.OBJDIR}/llvm_target/libcxx/lib/libc++.a \
		${.OBJDIR}/llvm_target/libcxx/lib/libc++abi.a \
		${.OBJDIR}/llvm_target/libcxx/lib/libc++experimental.a \
		${RAVYN_SDKROOT}/usr/lib/

RTOBJ = ${.OBJDIR}/llvm_target/compiler-rt/lib/ravynos/libclang_rt.osx.a
compiler_rt: runtime libunwind libcxx
	mkdir -p ${RTLIB:H} ${RTOBJ:H}
	${CC} -o ${.OBJDIR}/libcompiler_rt.dylib -fuse-ld=${LD} \
	  -shared -umbrella System \
	  -mmacosx-version-min=${MACOS_VERSION_MIN} \
	  -Wl,-install_name,/usr/lib/system/libcompiler_rt.dylib \
	  -Wl,-current_version,${LLVM_VERSION}  \
	  -Wl,-force_load,${RTOBJ} -Wl,-undefined,warning -Wl,-flat_namespace \
	  -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib -lsystem_platform \
	  -lsystem_c -lsystem_malloc -lsystem_m -ldispatch -ldyld -lsystem_kernel \
	  -lsystem_pthread -lunwind
	cp -f ${.OBJDIR}/libcompiler_rt.dylib ${RTLIB}

# We want a combined libc++ and libc++abi as our dylib. It needs libSystem
# for _dlopen and _dlsym. We do not link directly to libdyld.

CXXOBJ = ${.OBJDIR}/llvm_target/libcxx/lib/libc++.a
libcxx_dylib: libcxx
	mkdir -p ${CXXLIB:H} ${CXXOBJ:H}
	${CC} -o ${.OBJDIR}/libc++.dylib -dylib -shared \
	  -mmacos-version-min=${MACOS_VERSION_MIN} \
	  -fuse-ld=${LD} --sysroot=${RAVYN_SDKROOT} \
	  -Wl,-install_name,/usr/lib/libc++.dylib \
	  -Wl,-current_version,${LLVM_VERSION} \
	  -Wl,-force_load,${.OBJDIR}/llvm_target/libcxx/lib/libc++.a \
	  -Wl,-force_load,${.OBJDIR}/llvm_target/libcxx/lib/libc++abi.a \
	  -Wl,-force_load,${.OBJDIR}/llvm_target/libcxx/lib/libc++experimental.a \
	  ${RTOBJ} -L${RAVYN_SDKROOT}/usr/lib -lSystem 2>&1 | grep -v "addend too large"
	cp -f ${.OBJDIR}/libc++.dylib ${CXXLIB}

#add_dependencies(libdyld libsystem_malloc libsystem_c libsystem_pthread)
#add_dependencies(libSystem
#    xnu_headers libsystem_c libsystem_platform libsystem_pthread
#    libsystem_malloc libdyld libkernel libsystem_darwin
#    compiler_rt

.include <bsd.subdir.mk>
