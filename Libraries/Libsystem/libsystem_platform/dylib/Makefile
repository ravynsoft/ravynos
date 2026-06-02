LIB = system_platform
SEED_DEFS = -DLONG_BIT=64

DYLIB_LDFLAGS = --sysroot=${RAVYN_SDKROOT} -shared -dylib \
	-Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
        -mmacos-version-min=${MACOS_VERSION_MIN} -fuse-ld=${LD} \
        -Wl,-install_name,/usr/lib/system/libsystem_platform.dylib \
        -Wl,-current_version,220 -Wl,-compatibility_version,1.0.0 \
        -Wl,-search_paths_first -umbrella System \
        -Wl,-force_load,lib${LIB}.a \
        -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
        -lsystem_kernel -ldyld \
        -Wl,-alias_list,${.CURDIR:H}/xcodeconfig/libplatform.aliases

.PATH: ${.CURDIR}/..

all: lib${LIB}.dylib

lib${LIB}.dylib: lib${LIB}.a
	${CC} -o ${.TARGET} ${DYLIB_LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system
	cp -f lib${LIB}.dylib ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f lib${LIB}.dylib ${SYSROOT_DIR}/usr/lib/system/

.include "common.mk"

CFLAGS := ${CFLAGS:S/${.CURDIR}/${.CURDIR:H}/}

.include <bsd.lib.mk>
