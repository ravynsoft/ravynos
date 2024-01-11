.include <bsd.prog.mk>

.MAKEFLAGS=
CONFIGURE_ARGS+=        --enable-kqueue --enable-launchd
CONFIGURE_ARGS+=        --without-x --disable-tests
GMAKE_ARGS+=            LDFLAGS="-llaunch -ldispatch -lSystem"
#GMAKE_ARGS+=    INSTALL="${DESTDIR}/legacy/bin/install" CFLAGS="${CFLAGS}"

all: gnu_configure .WAIT gnu_build
build: gnu_build
clean: gnu_clean
install: gnu_install
