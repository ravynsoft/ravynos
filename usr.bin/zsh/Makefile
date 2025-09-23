.include <bsd.prog.mk>

PROG=                   zsh
CONFIGURE_ARGS+=        --bindir=/bin
CONFIGURE_ARGS+=        --enable-stack-allocation
CONFIGURE_ARGS+=        --enable-etcdir=${SYSCONFDIR}
CONFIGURE_ARGS+=        --enable-function-subdirs
CONFIGURE_ARGS+=        --enable-maildir-support
CONFIGURE_ARGS+=        --enable-multibyte
CONFIGURE_ARGS+=        --with-tcsetpgrp
CONFIGURE_ARGS+=        ${MACHINE_ARCH} ravynsoft

PERL5LIB=	${OBJTOP}/tmp/obj-tools/lib/perl5
LD_LIBRARY_PATH=	${OBJTOP}/tmp/obj-tools/lib
.export LD_LIBRARY_PATH PERL5LIB

all: zsh
zsh:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	cp -fv ${.CURDIR}/Doc/*.1 ${MAKEOBJDIR}
	${MAKE}
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${OBJTOP}/tmp/legacy/bin/install"

install:
	${MAKE} install DESTDIR=${DESTDIR} INSTALL="${OBJTOP}/tmp/legacy/bin/install"

includes:
