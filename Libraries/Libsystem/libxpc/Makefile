MK_WERROR = no
LIB = xpc
SRCS = mpack.c \
	xpc_array.c \
	xpc_connection.c \
	xpc_dictionary.c \
	xpc_misc.c \
	xpc_pipe.c \
	xpc_type.c \
	xpc_runtime.c \
	transports/mach.c

SLF = /System/Library/Frameworks
CFLAGS = -DXPC_DEBUG -DMACH -D__APPLE__ -g -O0 -fblocks \
	--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-isystem${RAVYN_SDKROOT}/usr/include/c++/v1 \
	-I${.CURDIR} -I${.CURDIR:H}/liblaunch -I${.CURDIR}/../../MiscLibs \
	-I${.CURDIR}/../../openbsm -I${RAVYN_SDKROOT}/usr/local/include \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
	-I${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/PrivateHeaders \
	-include${RAVYN_SDKROOT}/usr/include/stdatomic.h

LDFLAGS = -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
	-mmacos-version-min=${MACOS_VERSION_MIN}  -umbrella System \
	-L${CMAKE_CURRENT_BINARY_DIR}/../../MiscLibs -lsbuf -lnv \
	-L${RAVYN_SDKROOT}/usr/lib -lobjc \
	-L${RAVYN_SDKROOT}/usr/lib/system -ldispatch -lsystem_c \
	-lsystem_platform -lsystem_kernel -lsystem_blocks -lsystem_malloc \
	-llaunch -ldyld -lsystem_pthread \
	-Wl,-install_name,/usr/lib/system/libxpc.dylib \
	-Wl,-current_version,1738.0.17 \
	-Wl,-compatibility_version,1.0.0

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
