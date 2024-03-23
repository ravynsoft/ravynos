.include <bsd.prog.mk>

.MAKEFLAGS=
.if ${OBJTOP:M*obj-tools*} 
INSTALL=        ${OBJTOP}/../legacy/bin/install
.else
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
.endif

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g

FAKEACLOCAL= ${OBJTOP}/../usr/bin/aclocal-1.16

all: gnu_configure .WAIT build
build:
	ln ${OBJTOP}/tmp/legacy/bin/true ${FAKEACLOCAL}
	make
	rm -f ${FAKEACLOCAL}

install:
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install
