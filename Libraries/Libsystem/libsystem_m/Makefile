MK_WERROR = no
LIB = system_m
SRCS = math.c

CFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT}
LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-Wl,-install_name,/usr/lib/system/libsystem_m.dylib \
	-Wl,-current_version,3178 -Wl,-compatibility_version,1 \
	-Wl,-all_load -umbrella System -nostdlib

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
