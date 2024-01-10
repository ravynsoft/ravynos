.include <bsd.lib.mk>

all: build
build:
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

LD_ADD= libz liblzma libzstd
