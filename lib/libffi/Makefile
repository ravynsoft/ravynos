CONFIGURE_ARGS= --disable-docs
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install

all: build
build:
	${.CURDIR}/configure --prefix=${PREFIX} ${CONFIGURE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}

install:
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${INSTALL} DESTDIR=${DESTDIR} install

installconfig: .PHONY
includes: .PHONY

distribute:
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${INSTALL} \
		DESTDIR=${OBJTOP}/release/dist/base install
LD_ADD= 

