.include <bsd.prog.mk>

.MAKEFLAGS=
.if ${OBJTOP:M*obj-tools*} 
INSTALL=        ${OBJTOP}/../legacy/bin/install
.else
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
.endif

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g

all: gnu_configure .WAIT build
build:
	make

install:
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install
