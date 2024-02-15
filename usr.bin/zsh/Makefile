.include <bsd.prog.mk>

PROG=                   zsh
CONFIGURE_ARGS+=        --enable-stack-allocation
CONFIGURE_ARGS+=        --enable-etcdir=${SYSCONFDIR}
CONFIGURE_ARGS+=        --enable-function-subdirs
CONFIGURE_ARGS+=        --enable-maildir-support
CONFIGURE_ARGS+=        --enable-multibyte
CONFIGURE_ARGS+=        --with-tcsetpgrp
CONFIGURE_ARGS+=        ${MACHINE} ravynsoft


all: zsh
zsh:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE}
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

includes:
