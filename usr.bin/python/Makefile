.include <src.opts.mk>

PROG=                   python3.10
CONFIGURE_ARGS+=        --enable-optimizations
CONFIGURE_ARGS+=        --with-suffix=""

NEWCFLAGS=      -isystem=${DESTDIR}/usr/include/apple ${CFLAGS:N-Werror} -fPIC
NEWCFLAGS+=     -I${DESTDIR}/usr/include/apple/System -DHAVE_SYS_KERN_CONTROL_H

all: ${PROG} install
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE} CFLAGS="${NEWCFLAGS}"
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

install:
	${MAKE} -C ${.CURDIR}/Mako ${.TARGET}
	ln -sfv python3.10 ${DESTDIR}${PREFIX}/bin/python3
	ln -sfv python3 ${DESTDIR}${PREFIX}/bin/python

installconfig: .PHONY
includes: .PHONY

distribute:
	${MAKE} install DESTDIR=${DESTDIR} \
		INSTALL=${OBJTOP}/tmp/legacy/bin/install

.ORDER: ${PROG} install
