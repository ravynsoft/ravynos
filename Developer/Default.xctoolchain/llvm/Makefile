CC := ${HOST_CC}
CXX := ${HOST_CXX}

all: ${TOOLS}/clang

.if "${.MAKE.OS}" != "Darwin"
CFLAGS = -UHAVE_MACH_MACH_H -UAPPLE -U__APPLE__ -UHAVE_DECL_ARC4RANDOM
CXXFLAGS = ${CFLAGS} -U_GLIBCXX_USE_CXX11_ABI -D_GLIBCXX_USE_CXX11_ABI=0
.endif

${TOOLCHAIN}/usr/lib/libLLVM.a ${TOOLCHAIN}/usr/lib/libtapi.so \
${TOOLS}/clang ${TOOLS}/clang++: ${TOOLCHAIN}/usr/libexec/migcom ${.CURDIR}/Makefile
	cmake -S ${.CURDIR}/llvm -B ${.OBJDIR}/../llvm_host -Wno-dev \
	-DCMAKE_BUILD_TYPE=Release \
	-C${.CURDIR}/clang/cmake/caches/Apple-stage1.cmake \
	-DCMAKE_C_COMPILER=${HOST_CC} \
	-DCMAKE_CXX_COMPILER=${HOST_CXX} \
	-DCMAKE_C_FLAGS=${CFLAGS} \
	"-DCMAKE_CXX_FLAGS=${CXXFLAGS}" \
	-DCMAKE_CXX_STANDARD=17 \
	-DCMAKE_CXX_STANDARD_REQUIRED=ON \
	-DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}/usr \
	-DLLVM_BOOTSTRAP_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-darwin \
	-DLLVM_DEFAULT_TARGET_TRIPLE=${CpuArch}-apple-darwin \
	-DLLVM_TARGETS_TO_BUILD=${BuildArch} \
	-DLLVM_ENABLE_LTO=OFF \
	-DLLVM_APPEND_VC_REV=OFF \
	-DLLVM_ENABLE_PROJECTS="clang;tapi;lld" \
	-DLLVM_BUILD_RUNTIMES=ON \
	-DLLVM_BOOTSTRAP_ENABLE_LIBCXX=ON \
	-DLLVM_ENABLE_LIBCXX=OFF \
	-DLLVM_ENABLE_ZLIB=FORCE_ON \
	-DLLVM_BUILD_DOCS=OFF \
	-DLLVM_BUILD_TOOLS=OFF \
	-DLLVM_INCLUDE_TESTS=OFF \
	-DLLVM_INCLUDE_BENCHMARKS=OFF \
	-DLLVM_BUILD_EXAMPLES=OFF \
	-DLLVM_ENABLE_WARNINGS=OFF \
	-DLLVM_ENABLE_BINDINGS=OFF \
	-DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
	-DLLVM_INSTALL_BINUTILS_SYMLINKS=ON \
	-DLLVM_TOOL_LLVM_INSTALL_NAME_TOOL_BUILD=ON \
	-DCLANG_BUILD_TOOLS=ON \
	-DCLANG_BUILD_UTILS=ON \
	-DCLANG_ENABLE_STATIC_ANALYZER=OFF \
	-DCLANG_ENABLE_ARCMT=OFF \
	-DCLANG_ENABLE_FORMAT=OFF \
	-DCLANG_INCLUDE_TESTS=OFF \
	-DTAPI_INCLUDE_TESTS=OFF
	CC=${HOST_CC} CXX=${HOST_CXX} ${MAKE} -C${.OBJDIR}/../llvm_host all \
	   llvm-ar llvm-nm llvm-ranlib llvm-strip dsymutil llvm-lipo \
	   llvm-libtool-darwin llvm-objdump llvm-otool
	cp -Rf ${.OBJDIR}/../llvm_host/bin ${TOOLCHAIN}/usr/
	cp -Rf ${.OBJDIR}/../llvm_host/lib/clang ${TOOLCHAIN}/usr/lib/
	cp -Rf ${.OBJDIR}/../llvm_host/lib/lib* ${TOOLCHAIN}/usr/lib/
.for tool in ar nm ranlib lipo objdump libtool-darwin otool
	ln -sf llvm-${tool} ${TOOLCHAIN}/usr/bin/${tool}
.endfor
	ln -sf clang ${TOOLCHAIN}/usr/bin/cc
	ln -sf clang ${TOOLCHAIN}/usr/bin/c89
	ln -sf clang ${TOOLCHAIN}/usr/bin/c99
	ln -sf clang++ ${TOOLCHAIN}/usr/bin/c++
