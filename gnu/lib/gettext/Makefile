distribute:
.for i in . gettext-runtime gettext-tools libtextstyle
	ln -sfv ${OBJTOP}/tmp/legacy/bin/install \
		${MAKEOBJDIR}/${i}/install
.endfor
	PATH=$$PATH:${OBJTOP}/tmp/usr/bin \
		gmake -C ${.OBJDIR} MAKEFLAGS= \
		DESTDIR=${OBJTOP}/release/dist/base \
		install

.include <bsd.prog.mk>

.MAKEFLAGS=
PERL5LIB=	${OBJTOP}/tmp/obj-tools/lib/perl5
LD_LIBRARY_PATH+=	${OBJTOP}/tmp/obj-tools/lib
PATH=		${OBJTOP}/tmp/legacy/bin:/bin:/usr/bin
.export PERL5LIB
.export LD_LIBRARY_PATH
.export PATH


CFLAGS=         -I${DESTDIR}/../gnu/lib/gettext/libtextstyle/lib
LDFLAGS=	-L${OBJTOP}/tmp/usr/lib
CONFIGURE_ARGS+=        --enable-relocatable
INSTALL=	${OBJTOP}/tmp/legacy/bin/install
GMAKE_ARGS+=    INSTALL="${INSTALL}" CFLAGS="${CFLAGS}"
OO_FILES= \
	fd-ostream.oo.c \
	fd-ostream.oo.h \
	file-ostream.oo.c \
	file-ostream.oo.h \
	html-ostream.oo.c \
	html-ostream.oo.h \
	html-styled-ostream.oo.c \
	html-styled-ostream.oo.h \
	iconv-ostream.oo.c \
	iconv-ostream.oo.h \
	memory-ostream.oo.c \
	memory-ostream.oo.h \
	noop-styled-ostream.oo.c \
	noop-styled-ostream.oo.h \
	ostream.oo.c \
	ostream.oo.h \
	styled-ostream.oo.c \
	styled-ostream.oo.h \
	term-ostream.oo.c \
	term-ostream.oo.h \
	term-styled-ostream.oo.c \
	term-styled-ostream.oo.h 

all: gnu_configure .WAIT copy-header gnu_build
copy-header:
	cp -fv ${.CURDIR}/libtextstyle/lib/textstyle.h \
		${DESTDIR}/usr/include/textstyle.h
.for f in ${OO_FILES:S/.oo//g}
	cp -fv ${.CURDIR}/libtextstyle/lib/${f} ${MAKEOBJDIR}/
.endfor

build: gnu_build
clean: gnu_clean
install: gnu_install
