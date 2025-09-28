.include <bsd.lib.mk>

all: build
build:
	cmake -Djbig=OFF -DCMAKE_INSTALL_PREFIX=${PREFIX} \
		-DZSTD_INCLUDE_DIR=${SRCTOP}/sys/contrib/zstd/lib \
		-DZSTD_LIBRARY=${OBJTOP}/tmp/usr/lib/libprivatezstd.so.5 \
		-DZSTD_HAVE_DECOMPRESS_STREAM=TRUE -Dzstd=TRUE \
		-S ${.CURDIR} -B ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

LD_ADD= libz liblzma libzstd
