.include <bsd.prog.mk>

CONFIGURE_ARGS+=        --prefix=/usr
CONFIGURE_ARGS+=        --datadir=share/cmake
CONFIGURE_ARGS+=        --docdir=share/doc/cmake
CONFIGURE_ARGS+=        --mandir=share/man
CONFIGURE_ARGS+=        --no-qt-gui --qt-qmake=/bin/true

all: configure .WAIT build

configure:
	${.CURDIR}/configure ${CONFIGURE_ARGS}

build:
	make
	make install DESTDIR=${DESTDIR}

clean:
	rm -rf Bootstrap.cmk

install:
	make install DESTDIR=${DESTDIR}
