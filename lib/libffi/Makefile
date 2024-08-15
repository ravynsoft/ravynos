CONFIGURE_ARGS= --disable-docs
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install
RANLIB=		${OBJTOP}/gnu/usr.bin/binutils/binutils/ranlib
AR=		${OBJTOP}/gnu/usr.bin/binutils/binutils/ar

all: build
build:
	${.CURDIR}/configure --prefix=${PREFIX} ${CONFIGURE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}

install:
	cp -f ${RANLIB} ${AR} ${OBJTOP}/tmp/usr/bin
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${INSTALL} DESTDIR=${DESTDIR} install

installconfig: .PHONY
includes: .PHONY

distribute:
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${OBJTOP}/tmp/legacy/bin/install \
		DESTDIR=${OBJTOP}/release/dist/base install
LD_ADD= 

