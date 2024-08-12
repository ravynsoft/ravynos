.include <src.opts.mk>

PROG=                   python3.10
CONFIGURE_ARGS+=	--enable-shared --with-system-ffi --with-ensurepip --without-readline
CONFIGURE_ARGS+=	--without-system-libmpdec --with-hash-algorithm=siphash24
CONFIGURE_ARGS+=        --with-suffix="" --with-pymalloc --disable-test-modules
DESTDIR?=		${OBJTOP}/tmp
PREFIX?=		/usr

NEWCFLAGS=      -fPIC -std=c11
NEWCFLAGS+=     -I${DESTDIR}/usr/include/apple/System -DHAVE_SYS_KERN_CONTROL_H
.MAKEFLAGS=
.export .MAKEFLAGS

all: ${PROG} install
${PROG}:
	${.CURDIR}/configure --prefix=${PREFIX} --sysconfdir=/etc ${CONFIGURE_ARGS}
	${MAKE} CFLAGS="${NEWCFLAGS}" LDFLAGS="-lthr"
	${MAKE} DESTDIR=${DESTDIR} INSTALL="${DESTDIR}/legacy/bin/install" install

install:
	ln -sfv python3.10 ${DESTDIR}${PREFIX}/bin/python3
	ln -sfv python3 ${DESTDIR}${PREFIX}/bin/python
	${MAKE} -C ${.CURDIR}/Mako ${.TARGET}

installconfig: .PHONY
includes: .PHONY

distribute:
	${MAKE} install DESTDIR=${DESTDIR} \
		INSTALL=${OBJTOP}/tmp/legacy/bin/install

.ORDER: ${PROG} install
