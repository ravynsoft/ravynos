CONFIGURE_ARGS= --disable-docs
INSTALL=        ${OBJTOP}/tmp/legacy/bin/install

all: build
build:
	${.CURDIR}/configure --prefix=${PREFIX} ${CONFIGURE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}

install:
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${INSTALL} DESTDIR=${DESTDIR} install

includes: .PHONY

LD_ADD= 

