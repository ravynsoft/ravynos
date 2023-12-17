.include <bsd.prog.mk>

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g

all: gnu_configure .WAIT build
build:
	make

install: gnu_install

worldtmp_install:
	make install DESTDIR=${DESTDIR}/tmp/legacy/ BINDIR=/usr/bin
