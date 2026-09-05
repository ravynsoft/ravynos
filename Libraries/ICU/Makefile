MK_WERROR = no
LIB = icu

CFLAGS =--sysroot=${RAVYN_SDKROOT} -fuse-ld=${LD} \
	-mmacos-version-min=${MACOS_VERSION_MIN}
CXXFLAGS =-std=gnu++17 --sysroot=${RAVYN_SDKROOT} -fuse-ld=${LD} \
	-mmacos-version-min=${MACOS_VERSION_MIN}
LDFLAGS =--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-fuse-ld=${LD} -licudata -lc++ \
	${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Libraries/llvm_target/compiler-rt/lib/ravynos/libclang_rt.osx.a

EXTRA_FLAGS = --prefix=/usr --localstatedir=/var --sysconfdir=/etc

.if "${.MAKE.OS}" != "Darwin"
CFG_GUESS != sh ${.CURDIR}/source/config.guess
EXTRA_FLAGS += --with-cross-build=${.OBJDIR}/native \
	--host=${CpuArch}-apple-darwin --build=${CFG_GUESS} \
	--disable-samples --disable-tests --disable-extras
.endif

all: libicucore.A.dylib

lib${LIB}.a: ${.CURDIR}/Makefile
.if "${.MAKE.OS}" != "Darwin"
	mkdir -p ${.OBJDIR}/native
	cd ${.OBJDIR}/native; ${.CURDIR}/source/configure \
	  && MAKEFLAGS="" ${GMAKE}
.endif
	mkdir -p ${.OBJDIR}/target
	cd ${.OBJDIR}/target; ICU_CROSS_BUILD=1 SDKROOT=${RAVYN_SDKROOT} \
	  CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" LDFLAGS="${LDFLAGS}" \
	  CC=${CC} CXX=${CXX} LD=${LD} AR=${AR} STRIP=${STRIP} MAKEFLAGS="" \
	  ${.CURDIR}/source/runConfigureICU MacOSX ${EXTRA_FLAGS}
	MAKEFLAGS="" ${GMAKE} -C ${.OBJDIR}/target

libicucore.A.dylib: lib${LIB}.a
	${TOOLS}/install_name_tool -id /usr/lib/libicudata.dylib \
	  ${.OBJDIR}/target/lib/libicudata.dylib
	${CC} -v -o ${.TARGET} -shared -dylib -Wl,-all_load \
	  -Wl,-install_name,/usr/lib/${.TARGET} \
	  -Wl,-current_version,78.3 -Wl,-compatibility_version,1 \
	  $$(find ${.OBJDIR}/target/common ${.OBJDIR}/target/i18n \
	  ${.OBJDIR}/target/io -name '*.o') ${LDFLAGS}
.for dir in ${RAVYN_SDKROOT} ${SYSROOT_DIR}
	mkdir -p ${dir}/usr/lib ${dir}/usr/include
	cp -f ${.OBJDIR}/${.TARGET} ${dir}/usr/lib/
	ln -sf ${.TARGET} ${dir}/usr/lib/libicucore.dylib
	cp -fd ${.OBJDIR}/target/lib/libicudata* ${dir}/usr/lib/
	cp -Rf ${.CURDIR}/source/i18n/unicode ${.CURDIR}/source/i18n/*.h \
	  ${dir}/usr/include
.endfor
