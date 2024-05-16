.include <bsd.prog.mk>

PATH+=	${OBJTOP}/tmp/legacy/bin

all:
	cmake -G "Unix Makefiles" \
		-DCMAKE_SHARED_LINKER_FLAGS="-L${OBJTOP}/lib/libdl" \
		-DPKG_CONFIG_EXECUTABLE=/usr/bin/true \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
		-DLIBXML2_WITH_SAX1=ON \
		-DLIBXML2_WITH_PYTHON=OFF \
		-S ${.CURDIR} -B ${MAKEOBJDIR}
	make ${.MAKEFLAGS}
	make install DESTDIR=${DESTDIR}

install:
	make install DESTDIR=${DESTDIR}
