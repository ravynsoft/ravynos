.include <bsd.prog.mk>
.include "../../gnu/Makefile.inc"

.MAKEFLAGS=

PROG=                   nasm
CONFIGURE_ARGS+=        --prefix=${PREFIX}
CONFIGURE_ARGS+=        --enable-lto
GMAKE_ARGS+=            AR=ar

CONFIGURE_ENV+=		AUTOCONF=/usr/bin/true
CONFIGURE_ENV+=		AUTOHEADER=/usr/bin/true
CONFIGURE_ENV+=		AUTOMAKE=/usr/bin/true
CONFIGURE_ENV+=		ACLOCAL=/usr/bin/true
CONFIGURE_ENV+=		CFLAGS=-I${.CURDIR}

PERL5LIB=	${OBJTOP:S/\/tmp\/obj-tools//}/tmp/obj-tools/lib/perl5
LD_LIBRARY_PATH=	${OBJTOP:S/\/tmp\/obj-tools//}/tmp/obj-tools/lib
.export PERL5LIB
.export LD_LIBRARY_PATH

all: ${PROG}
copyfile:
	mkdir -p asm include
	cp -fv ${.CURDIR}/asm/warnings.c asm/
	echo '#!/bin/sh' > autogen.sh
	echo 'exit 0' >> autogen.sh
	chmod +x autogen.sh
	cp -fv ${.CURDIR}/configure .

${PROG}: gnu_configure .WAIT copyfile gnu_build .WAIT gnu_install
install: gnu_install

includes:

