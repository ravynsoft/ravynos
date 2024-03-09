.include <bsd.prog.mk>

#.MAKEFLAGS=
#GMAKE=                  ${DESTDIR}/legacy/bin/gmake

all: build
build:
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR}

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
