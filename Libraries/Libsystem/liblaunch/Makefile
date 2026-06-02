MK_WERROR = no
LIB = launch
.PATH: ${.OBJDIR}
SRCS = libbootstrap.c liblaunch.c libvproc.c \
	jobUser.c launchd_helperUser.c launchd_helperServer.c

COMMONFLAGS = --sysroot=${RAVYN_SDKROOT} -g -DPRIVATE \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-function-declaration -Wno-int-conversion \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd -I${.CURDIR} \
	-I${.CURDIR}/../../openbsm -I${.CURDIR:H}/libdispatch/config \
	-I${.CURDIR:H}/libsystem_pthread \
	-I${.CURDIR:H}/libsystem_c/fbsdcompat \
	-I${ROOT_SOURCE_DIR}/BSD/sbin/launchd \
	-I${RAVYN_SDKROOT}/usr/local/include \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc \
	-I${.CURDIR:H} -I${.OBJDIR} -I${.CURDIR:H}/private \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san

CFLAGS = ${COMMONFLAGS}
#	-include${RAVYN_SDKROOT}/usr/include/sys/_types/_mach_port_t.h \
	-include${RAVYN_SDKROOT}/usr/include/stdatomic.h

MIG_FLAGS = ${COMMONFLAGS} -D__MigTypeCheck

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} -umbrella System \
	-Wl,-current_version,1738.0.17 -Wl,-compatibility_version,1.0.0 \
	-Wl,-install_name,/usr/lib/system/liblaunch.dylib \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-lcompiler_rt -ldispatch -lsystem_c -lsystem_kernel -lsystem_malloc \
	-lsystem_asl -ldyld -lsystem_platform -lsystem_pthread

all: ${.OBJDIR}/lib${LIB}.dylib

helper.h launchd_helperUser.c launchd_helperServer.c: ${.CURDIR}/helper.defs
	${MIG} ${MIG_FLAGS} -header helper.h -sheader helperServer.h \
	  ${.CURDIR}/helper.defs

job.h jobUser.c: ${.CURDIR}/job.defs
	${MIG} ${MIG_FLAGS} -sheader jobServer.h ${.CURDIR}/job.defs

lib${LIB}.a: jobUser.c launchd_helperUser.c
${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
