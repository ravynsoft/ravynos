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

PERL5LIB=	${SRCTOP}/usr.bin/perl5/lib:${SRCTOP}/usr.bin/perl5/cpan:${OBJTOP}/lib/perl5
.export PERL5LIB

all: ${PROG}
copyfile:
	mkdir -p asm
	cp -fv ${.CURDIR}/asm/warnings.c asm/
	cp -fv ${.CURDIR}/autogen.sh .
	cp -fv ${.CURDIR}/configure .

${PROG}: gnu_configure .WAIT copyfile gnu_build .WAIT gnu_install
install: gnu_install

includes:

