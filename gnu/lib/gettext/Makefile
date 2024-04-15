.include <bsd.prog.mk>

.MAKEFLAGS=
PERL5LIB=	${OBJTOP}/tmp/obj-tools/lib/perl5
LD_LIBRARY_PATH=	${OBJTOP}/tmp/obj-tools/lib
PATH=		${PATH}:${OBJTOP}/tmp/legacy/bin
.export PERL5LIB
.export LD_LIBRARY_PATH
.export PATH

CFLAGS=         -I${DESTDIR}/../gnu/lib/gettext/libtextstyle/lib
CONFIGURE_ARGS+=        --enable-relocatable
GMAKE_ARGS+=    INSTALL="${DESTDIR}/legacy/bin/install" CFLAGS="${CFLAGS}"

all: gnu_configure .WAIT copy-header gnu_build
copy-header:
	cp -fv ${.CURDIR}/libtextstyle/lib/textstyle.h \
        ${DESTDIR}/usr/include/textstyle.h

build: gnu_build
clean: gnu_clean
install: gnu_install


