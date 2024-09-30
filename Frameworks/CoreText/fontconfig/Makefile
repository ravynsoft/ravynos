PATH=		${OBJTOP}/tmp/legacy/bin:/bin:/usr/bin:${MAKEOBJDIR}/../gperf/src
.export PATH

GMAKE=		${OBJTOP}/tmp/legacy/bin/gmake
INSTALL=	${OBJTOP}/tmp/legacy/bin/install

CONFIGURE_ARGS+= --prefix=/ \
		 --sysconfdir=/etc \
		 --with-templatedir=/System/Library/Frameworks/CoreText.framework/Resources \
		 --with-xmldir=/System/Library/Frameworks/CoreText.framework/Resources \
		 --localstatedir=/var \
		 --enable-static=yes \
		 --enable-shared=no \
		 --enable-libxml2 \
		 --disable-docs --disable-docbook \
		 --with-pic --disable-cache-build \
		 --with-sysroot=${OBJTOP}/tmp \
		 --with-default-fonts=/System/Library/Fonts,/Library/Fonts
FREETYPE_CFLAGS= -I${SRCTOP}/Frameworks/Onyx2D/freetype/include
FREETYPE_LIBS=	 -L${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework -lOnyx2D \
		 -Wl,-R/System/Library/Frameworks/Onyx2D.framework/Versions/A \
		 -L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		 -lFoundation -lobjc -lstdc++ \
		 -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A
LIBXML2_CFLAGS=  -I${SRCTOP}/lib/libxml2/include \
		 -I${OBJTOP}/lib/libxml2 \
		 -I${OBJTOP}/tmp/usr/include/libxml2
LIBXML2_LIBS=	 -L${OBJTOP}/lib/libxml2 -lxml2
LDFLAGS+=	 ${FREETYPE_LIBS} ${LIBXML2_LIBS}

LD_LIBRARY_PATH+= ${OBJTOP}/Frameworks/CFNetwork/CFNetwork.framework:${OBJTOP}/Frameworks/CoreFoundation/CoreFoundation.framework:${OBJTOP}/Frameworks/Foundation/Foundation.framework:${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework:${OBJTOP}/tmp/usr/lib
.export LD_LIBRARY_PATH FREETYPE_CFLAGS FREETYPE_LIBS
.export LIBXML2_CFLAGS LIBXML2_LIBS

all: build
${MAKEOBJDIR}/Makefile:
	mkdir -p ${MAKEOBJDIR}
	cd ${MAKEOBJDIR}; LDFLAGS="${LDFLAGS:S/-L /-L/Wg}" \
		PYTHON=${OBJTOP}/tmp/usr/bin/python3 \
		${.CURDIR}/configure ${CONFIGURE_ARGS}

build: ${MAKEOBJDIR}/Makefile
	PYTHON=${OBJTOP}/tmp/usr/bin/python3 ${GMAKE} -C ${MAKEOBJDIR}
	mkdir -pv ${BUILDROOT}/etc/fonts/conf.d
	cp -fv ${.CURDIR}/fonts.conf ${BUILDROOT}/etc/fonts/
	rm -f ${MAKEOBJDIR}/src/fcarch ${MAKEOBJDIR}/src/fcarch.o

clean:
	rm -rf ${MAKEOBJDIR}

install:
