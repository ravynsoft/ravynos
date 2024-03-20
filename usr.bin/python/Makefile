.include <bsd.prog.mk>

PROG=                   python
#CONFIGURE_ARGS+=        --enable-optimizations
CONFIGURE_ARGS+=        --with-suffix=""

all: ${PROG}
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE}
	#${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

includes:
