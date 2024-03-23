.include <bsd.prog.mk>

.MAKEFLAGS=
.if ${OBJTOP:M*obj-tools*} 
INSTALL=        ${OBJTOP}/../legacy/bin/install
FAKEACLOCAL=    ${OBJTOP}/../usr/bin/aclocal-1.16
.else
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
FAKEACLOCAL=    ${OBJTOP}/tmp/usr/bin/aclocal-1.16
.endif

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g


all: gnu_configure .WAIT build
build:
	mkdir -p ${FAKEACLOCAL:H}
	echo '#!/bin/sh' >${FAKEACLOCAL}
	echo 'exit 0' >>${FAKEACLOCAL}
	chmod +x ${FAKEACLOCAL}
	make

install:
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install
	rm -f ${FAKEACLOCAL}

