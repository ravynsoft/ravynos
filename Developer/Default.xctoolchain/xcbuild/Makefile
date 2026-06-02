TARGETS = ${TOOLS}/xcbuild ${TOOLS}/xcodebuild ${TOOLS}/xcrun

.if "${.MAKE.OS}" == "Darwin"
RPATH = -rpath,@executable_path
.else
RPATH = -R$$ORIGIN
.endif

all: ${TOOLS}/xcbuild

${TARGETS}:
	cmake -S ${.CURDIR} -B ${.OBJDIR} -Wno-dev \
	-DCMAKE_C_COMPILER=${HOST_CC} \
	-DCMAKE_CXX_COMPILER=${HOST_CXX} \
	-DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR} \
	-DROOT_BINARY_DIR=${ROOT_BINARY_DIR} \
	'-DCMAKE_BUILD_RPATH=${RPATH}/../lib' \
	'-DCMAKE_INSTALL_RPATH=${RPATH}/../lib' \
	-DCMAKE_INSTALL_PREFIX=${TOOLCHAIN}
	CC=${HOST_CC} CXX=${HOST_CXX} make -C${.OBJDIR}
	CC=${HOST_CC} CXX=${HOST_CXX} make -C${.OBJDIR} install

