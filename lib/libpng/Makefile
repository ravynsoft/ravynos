.include <bsd.lib.mk>

CMAKE=	${OBJTOP}/usr.bin/cmake/bin/cmake
.if ${MACHINE} == "amd64"
CMAKE_ARGS+=    -DCMAKE_SYSTEM_PROCESSOR=x86_64
.endif

CPPFLAGS= --sysroot=${OBJTOP} ${CFLAGS:M-I*}
CFLAGS+= -Wno-error=unused-but-set-variable
CFLAGS+= -Wno-error=null-pointer-subtraction
CFLAGS+= -Wno-error=missing-variable-declarations
CFLAGS+= -Wno-error=cast-align
CFLAGS+= -Wno-error=cast-qual
LDFLAGS+= -lz -llzma -lprivatezstd

all: build
build:
	${CMAKE} -S ${.CURDIR} -B ${MAKEOBJDIR} \
		-DCMAKE_C_FLAGS="${CFLAGS:N-I*}" \
		-DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
		-DPNG_BUILD_ZLIB=off \
		-DZLIB_INCLUDE_DIR="${OBJTOP}/tmp/usr/include" \
		-DZLIB_ROOT="${OBJTOP}/tmp" \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
                ${CMAKE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:

distribute-me:
	mkdir -p ${OBJTOP}/release/dist/ravynOS
	${MAKE} -C ${MAKEOBJDIR} INSTALL=${INSTALL} \
		DESTDIR=${OBJTOP}/release/dist/ravynOS install
distribute: distribute-me
