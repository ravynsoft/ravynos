.include <src.opts.mk>

MK_WERROR=no
WARNS=1
CFLAGS+= --sysroot=${OBJTOP}/tmp -B{$OBJTOP}/tmp/usr/bin
CMAKE?= ${OBJTOP}/tmp/usr/bin/cmake

.include <bsd.lib.mk>


all: build
build:
	${CMAKE} -S ${.CURDIR} -B ${MAKEOBJDIR} \
		-DCMAKE_C_FLAGS="${CFLAGS}" \
		-DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=${PREFIX} \
		-DBUILD_SHARED_LIBS=true \
		-DFT_DISABLE_BZIP2=TRUE \
		-DFT_DISABLE_HARFBUZZ=TRUE \
		-DFT_DISABLE_BROTLI=TRUE \
		-DSKIP_INSTALL_ALL=TRUE \
		-DZLIB_FOUND=TRUE -DPNG_FOUND=TRUE \
		-DZLIB_LIBRARIES="-L${OBJTOP}/lib/libz -lz" \
		-DPNG_LIBRARY="-L${OBJTOP}/lib/libpng -lpng" \
		-DPNG_PNG_INCLUDE_DIR="${SRCTOP}/lib/libpng"
	${MAKE} -C ${MAKEOBJDIR}

install:
includes:
clean:

