.PATH: ${.CURDIR:H}

LIB = pthread

.include "common.mk"

CFLAGS := ${CFLAGS:S/${.CURDIR}/${.CURDIR:H}/} -DVARIANT_STATIC -DVARIANT_DYLD

.include <bsd.lib.mk>
