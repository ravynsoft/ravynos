.include <bsd.prog.mk>

.MAKEFLAGS=
INSTALL=        "${DESTDIR}/bin/install"
# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g

all: gnu_configure .WAIT build
build:
	make
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install

install: GMAKE_ARGS="INSTALL=${INSTALL}" gnu_install

#worldtmp_install:
#	make install BINDIR=/usr/bin
