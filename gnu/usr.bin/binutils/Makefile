.include <bsd.prog.mk>

.MAKEFLAGS=
# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g
CONFIGURE_ARGS+=        --enable-lto
GMAKE=                  ${DESTDIR}/legacy/bin/gmake

all: gnu_configure .WAIT build
build: gnu_build
install: gnu_install

worldtmp_install:
	make install DESTDIR=${DESTDIR}/legacy/ BINDIR=/usr/bin
