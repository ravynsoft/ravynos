.include <bsd.prog.mk>

PATH+=	${OBJTOP}/tmp/legacy/bin

all:
	cmake -G "Unix Makefiles" \
		-DPKG_CONFIG_EXECUTABLE=/usr/bin/true \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
		-DLIBXML2_WITH_SAX1=ON \
		-S ${.CURDIR} -B ${MAKEOBJDIR}
	make ${.MAKEFLAGS}
	make install DESTDIR=${DESTDIR}

install:
	make install DESTDIR=${DESTDIR}
