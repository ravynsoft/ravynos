MK_WERROR = no
SUBDIR =

all:
	CC=${CC} CXX=${CXX} LD=${CC} \
	LDFLAGS="-fuse-ld=${LD} --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	CFLAGS="--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	cmake -G "Unix Makefiles" \
		-DCMAKE_SHARED_LINKER_FLAGS="-L${OBJTOP}/lib/libdl" \
		-DPKG_CONFIG_EXECUTABLE=/usr/bin/true \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DLIBXML2_WITH_SAX1=ON \
		-DLIBXML2_WITH_PYTHON=OFF \
		-S ${.CURDIR} -B ${.OBJDIR:H}/xml2
	make ${.MAKEFLAGS}
	make install DESTDIR=${SYSROOT_DIR}
	make install DESTDIR=${RAVYN_SDKROOT}

.include <rvn.framework.mk>
.include <bsd.subdir.mk>
