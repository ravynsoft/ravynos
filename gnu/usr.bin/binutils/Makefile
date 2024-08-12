distribute:
	rm -f ${OBJTOP}/tmp/legacy/bin/ar
	ln -f ${OBJTOP}/usr.bin/ar/ar \
		${OBJTOP}/tmp/legacy/bin/ar
	ln -sfv ${OBJTOP}/tmp/legacy/bin/install \
		${MAKEOBJDIR}/install
	PATH=$$PATH:${OBJTOP}/tmp/usr/bin \
		gmake -C ${.OBJDIR} MAKEFLAGS= \
		DESTDIR=${OBJTOP}/release/dist/base \
		INSTALL=install install

.include <bsd.prog.mk>

.MAKEFLAGS=
# Ensure we call it gmake to avoid clash with BSD make!
CONFIGURE_ARGS+=        --enable-lto \
			--program-prefix=g \
			--with-build-sysroot=${OBJTOP}/tmp
DESTDIR?= 		${OBJTOP}/tmp
GMAKE=                  ${DESTDIR}/legacy/bin/gmake

all: gnu_configure .WAIT build
build: gnu_build
install: gnu_install

worldtmp_install:
	make install DESTDIR=${DESTDIR}/legacy/ BINDIR=/usr/bin
