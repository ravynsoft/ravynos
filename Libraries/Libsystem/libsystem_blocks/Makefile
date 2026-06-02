LIB = system_blocks
SRCS = data.c runtime.c

MK_WERROR = no

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -momit-leaf-frame-pointer -w \
        -Wno-implicit-function-declaration \
        -Wno-int-conversion -Wno-implicit-int \
        -fno-stack-protector -fno-stack-check -fno-builtin \
        -I${.CURDIR} -I${.CURDIR}/../private -I${ROOT_BINARY_DIR}/${CpuArch}

LDFLAGS = --sysroot=${RAVYN_SDKROOT} \
    -umbrella System -L${RAVYN_SDKROOT}/usr/lib \
    -Wl,-install_name,/usr/lib/system/libsystem_blocks.dylib \
    -Wl,-search_paths_first -mmacosx-version-min=${MACOS_VERSION_MIN} \
    -Wl,-current_version,74 -Wl,-compatibility_version,1.0.0 \
    -L${RAVYN_SDKROOT}/usr/lib/system -ldispatch -lsystem_malloc -lsystem_c \
    -ldyld

all: lib${LIB}.dylib
lib${LIB}.dylib: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -shared -dylib -o ${.TARGET} \
		-Wl,-force_load,${.ALLSRC} ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
