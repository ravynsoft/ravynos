GMAKE=		${OBJTOP}/tmp/legacy/bin/gmake
INSTALL=	${OBJTOP}/tmp/legacy/bin/install

CFLAGS=		--sysroot=${OBJTOP}/tmp
CXXFLAGS=	${CFLAGS} -std=c++11
.export CFLAGS CXXFLAGS

CONFIGURE_ARGS+= --prefix=/usr \
		 --sysconfdir=/etc \
		 --localstatedir=/var
all: build
configure: .EXEC
	mkdir -p ${MAKEOBJDIR}
	cd ${MAKEOBJDIR}; ${.CURDIR}/configure ${CONFIGURE_ARGS}

build: configure
	${GMAKE} -C ${MAKEOBJDIR}

clean:
	rm -rf ${MAKEOBJDIR}

install: .PHONY
