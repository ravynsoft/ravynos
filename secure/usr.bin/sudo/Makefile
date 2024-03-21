.include <bsd.prog.mk>

PROG=                   sudo
CONFIGURE_ARGS+=        --disable-root-mailer --enable-shell-sets-home
CONFIGURE_ARGS+=        --enable-pie --disable-leaks --enable-static-sudoers
CONFIGURE_ARGS+=        --disable-shared-libutil --disable-log-server
CONFIGURE_ARGS+=        --disable-log-client --with-bsm-audit --with-logincap
CONFIGURE_ARGS+=        --without-lecture --with-logging=syslog
CONFIGURE_ARGS+=        --without-sendmail 
CONFIGURE_ARGS+=        ${MACHINE} ravynsoft


all: ${PROG}
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE} CFLAGS="-fblocks" LDFLAGS="-lSystem"
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install"

includes:
