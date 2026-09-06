all: ${.OBJDIR}/Makefile build

${.OBJDIR}/Makefile: ${.CURDIR}/configure
	CC=${CC} CXX=${CXX} LD=${CC} AR=${AR} RANLIB=${RANLIB} LIBTOOL=${LIBTOOL} \
	LDFLAGS="-fuse-ld=${LD} --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	CFLAGS="--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	${.CURDIR}/configure --prefix=/usr --sysconfdir=/etc \
        --localstatedir=/var --host=${CpuArch}-apple-darwin \
        --disable-doc --disable-nls --disable-rpath  \
        --with-sysroot=${RAVYN_SDKROOT}

build: ${.OBJDIR}/Makefile
	MAKEFLAGS="" ${GMAKE} -C ${.OBJDIR}
	cp -f ${.OBJDIR}/src/liblzma/.libs/liblzma.5.dylib \
            ${.OBJDIR:H}/CoreText.framework/Resources/
