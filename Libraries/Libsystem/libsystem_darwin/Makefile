MK_WERROR = no
LIB = system_darwin
SRCS = bsd.c \
	ctl.c \
	dirstat.c \
	dirstat_collection.c \
	err.c \
	exception.c \
	init.c \
	mach.c \
	stdio.c \
	stdlib.c \
	string.c \
	variant.c

SLF = /System/Library/Frameworks
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-DTARGET_OS_BRIDGE=0 -D__DARWIN_UNIX03 -Dguardid_t=uint64_t \
	-D_LIBC_NO_FEATURE_VERIFICATION -DDARWIN_BUILDING_LIBSYSTEM_DARWIN \
	-DOS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE -DMAC_OS_X_VERSION_MIN_REQUIRED=1150 \
	-Wno-implicit-function-declaration -Wno-nullability-completeness \
	-Wno-int-conversion -Wno-implicit-int \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${.CURDIR:H}/libsystem_c/include \
	-I${.CURDIR:H}/libsystem_c/locale \
	-I${.CURDIR:H}/libsystem_c/locale/FreeBSD \
	-I${.CURDIR:H}/libsystem_c/stdtime/FreeBSD \
	-I${.CURDIR:H}/libsystem_c \
	-I${.CURDIR:H}/liblaunch \
	-I${.CURDIR:H}/libdispatch/private \
	-I${.CURDIR:H}/private \
	-I${.CURDIR}/h \
	-I${.CURDIR}/../../openbsm \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
	-include${RAVYN_SDKROOT}/usr/include/os/availability.h \
	-include${.CURDIR}/../../openbsm/sys/bsm/audit.h

LDFLAGS = --target=${CpuArch}-apple-darwin -Wl,-syslibroot,${RAVYN_SDKROOT} \
	-mmacos-version-min=${MACOS_VERSION_MIN} -nostdlib -nodefaultlibs \
	-Wl,-current_version,10.15.0 -Wl,-compatibility_version,1.0.0 \
	-Wl,-search_paths_first -umbrella System \
	-Wl,-install_name,/usr/lib/system/libsystem_darwin.dylib \
	-L${RAVYN_SDKROOT}/usr/lib/system -lsystem_kernel \
	-L${.OBJDIR}/../../dyld -ldyld -lsystem_malloc -lsystem_platform \
	-lsystem_pthread -lsystem_c -lunwind -ldyld -lm -ldispatch -lxpc

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: headers lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

headers:
	mkdir -p ${RAVYN_SDKROOT}/usr/local/include/os
	cp -f ${.CURDIR}/h/*.h ${RAVYN_SDKROOT}/usr/local/include/os/
.for path in ${RAVYN_SDKROOT} ${SYSROOT_DIR}
	mkdir -p ${path}/System/Library/CoreServices
	cp -f ${.CURDIR}/AppleFactoryVariant.plist \
	  ${.CURDIR}/AppleInternalVariant.plist \
	  ${path}/System/Library/CoreServices/
.endfor

.include <bsd.lib.mk>
