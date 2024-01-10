.include <bsd.prog.mk>
.include "../../gnu/Makefile.inc"

PROG=                   nasm
CONFIGURE_ARGS+=        --prefix=${PREFIX}
CONFIGURE_ARGS+=        --enable-lto
GMAKE_ARGS+=            AR=ar

all: ${PROG}
${PROG}: gnu_configure .WAIT gnu_build .WAIT gnu_install
install: gnu_install

includes:

