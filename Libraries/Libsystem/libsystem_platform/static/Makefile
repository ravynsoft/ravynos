SEED_DEFS = -DVARIANT_STATIC -DVARIANT_NO_RESOLVERS -DVARIANT_DYLD
LIB = system_platform
.PATH: ${.CURDIR}/..

all: libplatform.a
libplatform.a: lib${LIB}.a
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib/dyld
	cp -f ${.OBJDIR}/lib${LIB}.a \
	  ${RAVYN_SDKROOT}/usr/local/lib/dyld/${.TARGET}

.include "../common.mk"

# Fix the include paths to be relative to our parent dir
CFLAGS := ${CFLAGS:S/${.CURDIR}/${.CURDIR:H}/}

.include <bsd.lib.mk>
