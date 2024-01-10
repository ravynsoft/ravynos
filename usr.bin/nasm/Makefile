.include <bsd.prog.mk>
.include "../../gnu/Makefile.inc"

.MAKEFLAGS=

PROG=                   nasm
CONFIGURE_ARGS+=        --prefix=${PREFIX}
CONFIGURE_ARGS+=        --enable-lto
GMAKE_ARGS+=            AR=ar

all: ${PROG}
copyfile:
	mkdir -p asm
	cp -fv ${.CURDIR}/asm/warnings.c asm/

${PROG}: gnu_configure .WAIT copyfile gnu_build .WAIT gnu_install
install: gnu_install

includes:

