MK_WERROR = no
LIB = system_notify
SRCS = libnotify.c notify_client.c table.c

.PATH: ${.OBJDIR}
SRCS += notify_ipcUser.c
SLF = /System/Library/Frameworks

# FIXME: zoe 2/10/26 - Should generate notify_probes.h from .d with dtrace

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-DPRIVATE -D__APPLE__ -fblocks -DDISPATCH_USE_DTRACE=0 \
	-DTARGET_OS_BRIDGE=0 -DNOTIFY_POST=notify_post \
	-DNOTIFY_CHECK=notify_check -DNOTIFY_DELIVER_END=deliver_end \
	-DNOTIFY_DELIVER_START=deliver_start \
	-DNOTIFY_REGISTER_MACH_PORT=notify_register_mach_port \
	-w -Wno-int-conversion -Wno-implicit-int \
	-I${.CURDIR}/../liblaunch \
	-I${.CURDIR}/../private \
	-I${.CURDIR}/../libsystem_c \
	-I${.CURDIR}/../libdispatch/private  \
	-I${.OBJDIR} \
	-I${ROOT_BINARY_DIR}/${CpuArch} \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
#    "-include${RAVYN_SDKROOT}/usr/include/xpc/xpc.h"
#    "-include${RAVYN_SDKROOT}/usr/include/xpc/launchd.h"

MIG_FLAGS = ${CFLAGS} -D__MigTypeCheck

LDFLAGS = -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -umbrella System \
	-L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system \
	-Wl,-current_version,241 -Wl,-compatibility_version,1 \
	-Wl,-install_name,/usr/lib/system/libsystem_notify.dylib \
	-mmacos-version-min=${MACOS_VERSION_MIN} -lxpc -lsystem_blocks \
	-lsystem_c -lsystem_kernel -lsystem_platform -lsystem_asl \
	-lsystem_malloc -ldispatch -llaunch -ldyld

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/notify_ipc.h ${.OBJDIR}/notify_ipcUser.c \
	${.OBJDIR}/notify_ipcServer.c: ${.CURDIR}/notify_ipc.defs
	${MIG} ${MIG_FLAGS} -novouchers -header notify_ipc.h \
		${.CURDIR}/notify_ipc.defs

lib${LIB}.a: ${.OBJDIR}/notify_ipcUser.c

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
		-Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
