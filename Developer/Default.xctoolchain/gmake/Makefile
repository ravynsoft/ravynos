distribute: gnu_distribute

.include <bsd.prog.mk>

.MAKEFLAGS=
.if ${OBJTOP:M*obj-tools*} 
INSTALL=        ${OBJTOP}/../legacy/bin/install
.else
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
.endif
DESTDIR?= ${OBJTOP}/tmp

# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --program-prefix=g

# Fake out missing autoconf tools
CONFIGURE_ENV+=         ACLOCAL=/usr/bin/true
CONFIGURE_ENV+=         AUTOCONF=/usr/bin/true
CONFIGURE_ENV+=         AUTOMAKE=/usr/bin/true
CONFIGURE_ENV+=         AUTOHEADER=/usr/bin/true
CONFIGURE_ENV+=         MAKEINFO=/usr/bin/true
#CONFIGURE_ENV+=		LDFLAGS="-framework Foundation"

all: gnu_configure .WAIT build
build:
	make

install:
	${MAKEOBJDIR}/make INSTALL=${INSTALL} DESTDIR=${DESTDIR} install

