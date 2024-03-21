.include <bsd.prog.mk>

PROG=                   python
CONFIGURE_ARGS+=        --enable-optimizations
CONFIGURE_ARGS+=        --with-suffix=""

NEWCFLAGS=      -isystem=${DESTDIR}/usr/include/apple ${CFLAGS:N-Werror} -fPIC
NEWCFLAGS+=     -I${DESTDIR}/usr/include/apple/System -DHAVE_SYS_KERN_CONTROL_H

all: ${PROG}
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE} CFLAGS="${NEWCFLAGS}"

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"
	${MAKE} -C Mako ${.TARGET}

includes:
