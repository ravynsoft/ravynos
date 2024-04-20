.include <bsd.lib.mk>

CMAKE=	${OBJTOP}/usr.bin/cmake/bin/cmake
.if ${MACHINE} == "amd64"
CMAKE_ARGS+=    -DCMAKE_SYSTEM_PROCESSOR=x86_64
.endif

CPPFLAGS= --sysroot=${OBJTOP} ${CFLAGS:M-I*} 
CFLAGS+= -Wno-error=unused-but-set-variable
CFLAGS+= -Wno-error=null-pointer-subtraction
CFLAGS+= -Wno-error=missing-variable-declarations
LDFLAGS+= -lz -llzma -lprivatezstd

all: build
build:
	${CMAKE} -S ${.CURDIR} -B ${MAKEOBJDIR} \
		-DCMAKE_C_FLAGS="${CFLAGS:N-I*}" \
		-DPNG_BUILD_ZLIB=${SRCTOP}/contrib/zlib \
		-DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
		-DZLIB_LIBRARY="-L${OBJTOP}/lib/libzlib -lz"\
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
                ${CMAKE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
