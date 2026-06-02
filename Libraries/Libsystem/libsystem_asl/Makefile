MK_WERROR = no
LIB = system_asl
SRCS =  asl.c \
	asl_client.c \
	asl_core.c \
	asl_fd.c \
	asl_file.c \
	asl_legacy1.c \
	asl_msg.c \
	asl_msg_list.c \
	asl_mt_shim.c \
	asl_object.c \
	asl_store.c \
	asl_string.c \
	asl_util.c \
	syslog.c \
	asl_ipcUser.c

SLF = /System/Library/Frameworks
CFLAGS = -DXPC_DEBUG -DMACH -D__APPLE__ -D__DARWIN_C_LEVEL=__DARWIN_C_FULL \
	-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -fblocks \
	--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-function-declaration \
	-isystem${RAVYN_SDKROOT}/usr/include/c++/v1 -I${.CURDIR} \
	-I${.CURDIR:H}/liblaunch -I${.CURDIR}/../../MiscLibs \
	-I${.CURDIR}/../../openbsm \
	-I${.CURDIR:H}/libsystem_info/membership.subproj \
	-I${ROOT_SOURCE_DIR}/BSD/syslog/aslcommon \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${RAVYN_SDKROOT}/usr/local/include \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
	-I${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/PrivateHeaders \
	-I${.CURDIR:H}/private -I${.OBJDIR} \
	-include${RAVYN_SDKROOT}/usr/include/stdatomic.h

MIG_FLAGS = -DPRIVATE -DXPC_DEBUG -DMACH -D__APPLE__ -fblocks \
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}

asl_ipc.h asl_ipcUser.c: ${.CURDIR}/asl_ipc.defs
	${MIG} ${MIG_FLAGS} -header asl_ipc.h ${.CURDIR}/asl_ipc.defs

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} -umbrella System \
	-Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-lCrashReporterClient -ldyld -lsystem_kernel -lsystem_platform \
	-lsystem_pthread -lsystem_malloc -lsystem_c -lsystem_trace \
	-lsystem_notify -ldispatch -lxpc -llaunch -Wl,-upward-lsystem_info \
	-Wl,-install_name,/usr/lib/system/libsystem_asl.dylib \
	-Wl,-current_version,377.0.1 -Wl,-compatibility_version,1.0.0

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: asl_ipcUser.c lib${LIB}.a
	${CC} -shared -dylib -fuse-ld=${LD} -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

.include <bsd.lib.mk>
