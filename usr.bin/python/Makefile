.include <bsd.prog.mk>

PROG=                   python
CONFIGURE_ARGS+=        --enable-optimizations
CONFIGURE_ARGS+=        --with-suffix=""

CFLAGS+=        -I${DESTDIR}/usr/include/apple/

all: ${PROG}
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE}

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

includes:
