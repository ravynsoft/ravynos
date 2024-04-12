.include <bsd.lib.mk>

.if ${MACHINE} == "amd64"
CMAKE_ARGS+=    -DCMAKE_SYSTEM_PROCESSOR=x86_64
.endif

CPPFLAGS=	${CFLAGS:M-I*}
CFLAGS=		${CFLAGS:N-I*}

all: build
build:
	echo CPPFLAGS ${CPPFLAGS}
	echo CFLAGS ${CFLAGS}
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR} \
		-DPNG_BUILD_ZLIB=${SRCTOP}/contrib/zlib \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
                ${CMAKE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
