.include <bsd.lib.mk>

all: build
build:
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR} -DCMAKE_INSTALL_PREFIX=${PREFIX} \
                -DCMAKE_INSTALL_DATAROOTDIR=share/libjpeg \
                -DCMAKE_INSTALL_DOCDIR=share/doc/libjpeg \
                -DCMAKE_INSTALL_INCLUDEDIR=include \
	        -DCMAKE_INSTALL_LIBDIR=lib \
                -DCMAKE_INSTALL_BINDIR=bin \
                -DCMAKE_INSTALL_MANDIR=share/man
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
