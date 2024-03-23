.include <bsd.prog.mk>

.MAKEFLAGS=
.if ${OBJTOP:M*obj-tools*} 
INSTALL=        ${OBJTOP}/../legacy/bin/install
FAKEACLOCAL=    ${OBJTOP}/../usr/bin/aclocal-1.16
TRUE=           ${OBJTOP}/../legacy/bin/true
.else
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
FAKEACLOCAL=    ${OBJTOP}/tmp/usr/bin/aclocal-1.16
TRUE=           ${OBJTOP}/tmp/legacy/bin/true
.endif

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g


all: gnu_configure .WAIT build
build:
	ln ${TRUE} ${FAKEACLOCAL}
	make
	rm -f ${FAKEACLOCAL}

install:
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install
