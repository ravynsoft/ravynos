.include <bsd.prog.mk>

#.MAKEFLAGS=
#GMAKE=                  ${DESTDIR}/legacy/bin/gmake

all: build
build:
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR} -DCMAKE_INSTALL_PREFIX=${PREFIX}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
