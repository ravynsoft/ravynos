.include <bsd.prog.mk>

.MAKEFLAGS=
CONFIGURE_ARGS+=        --enable-kqueue --enable-launchd
CONFIGURE_ARGS+=        --without-x --disable-tests
LDFLAGS=                -L${OBJTOP}/lib/libexpat
LDFLAGS+=               -llaunch -ldispatch -lSystem
GMAKE_ARGS+=            CFLAGS="-I${SRCTOP}/contrib/expat/lib"
GMAKE_ARGS+=            LDFLAGS="${LDFLAGS}" EXPAT_LIBS=-lbsdxml

all: gnu_configure .WAIT gnu_build
build: gnu_build
clean: gnu_clean
install: gnu_install
