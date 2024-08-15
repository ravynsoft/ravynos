.include <bsd.prog.mk>

CONFIGURE_ARGS+=        --prefix=${OBJTOP}/tmp/usr
CONFIGURE_ARGS+=	--init=${.CURDIR}/ravyn_init.txt
CONFIGURE_ARGS+=        --datadir=share/cmake
CONFIGURE_ARGS+=        --docdir=share/doc/cmake
CONFIGURE_ARGS+=        --mandir=share/man
CONFIGURE_ARGS+=        --no-qt-gui --qt-qmake=/bin/true

PKG_CONFIG_SYSROOT_DIR=	${OBJTOP}/tmp
.export PKG_CONFIG_SYSROOT_DIR
DESTDIR?= ${OBJTOP}/tmp

all: configure .WAIT build
build-tools: all

configure:
	${.CURDIR}/configure ${CONFIGURE_ARGS}

build:
	make
	make install DESTDIR=${DESTDIR}

clean:
	rm -rf Bootstrap.cmk

install:
	make install DESTDIR=${DESTDIR}
