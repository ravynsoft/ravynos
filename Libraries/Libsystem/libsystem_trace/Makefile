MK_WERROR = no
LIB = system_trace
SRCS = log.c

SLF = /System/Library/Frameworks
CFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-fblocks -DOS_OBJECT_USE_OBJC \
	-I${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/Headers \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern/libkern

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-install_name,/usr/lib/system/libsystem_trace.dylib \
	-Wl,-current_version,1147.0.3 -Wl,-compatibility_version,1 \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-umbrella System -lobjc -lsystem_c -lsystem_platform -ldispatch \
	-ldyld

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system

.include <bsd.lib.mk>
