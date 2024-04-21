.include <bsd.prog.mk>

.MAKEFLAGS=
EXPAT_CFLAGS=           -I${SRCTOP}/contrib/expat/lib
EXPAT_LIBS=             -lbsdxml
CONFIGURE_ARGS+=        --enable-kqueue --enable-launchd
CONFIGURE_ARGS+=        --without-x --disable-tests
CFLAGS=                 --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
CFLAGS+=		-I${SRCTOP}/lib/liblaunch
LDFLAGS=                -L${OBJTOP}/lib/libexpat
LDFLAGS+=               -llaunch -ldispatch -lSystem
GMAKE_ARGS+=            LDFLAGS="${LDFLAGS}" LIBS="${EXPAT_LIBS}"
GMAKE_ARGS+=		INSTALL=${.CURDIR}/build-aux/install-sh
LAUNCHCTL=		${OBJTOP}/bin/launchctl/launchctl

.export EXPAT_CFLAGS
.export EXPAT_LIBS
.export CFLAGS
.export LAUNCHCTL

fake-ranlib:
	ln ${OBJTOP}/usr.bin/ar/ar \
		${OBJTOP}/tmp/legacy/bin/ranlib

all: gnu_configure .WAIT gnu_build
build: gnu_build
clean: gnu_clean
install: gnu_install
distribute: fake-ranlib
