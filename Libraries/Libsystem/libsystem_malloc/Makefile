LIB = system_malloc
SRCS = \
	bitarray.c empty.s frozen_malloc.c legacy_malloc.c \
	magazine_large.c magazine_malloc.c magazine_medium.c \
	magazine_rack.c magazine_small.c magazine_tiny.c \
	malloc_common.c malloc_printf.c malloc.c nano_malloc.c \
	msl_lite_support.c nano_malloc_common.c nanov2_malloc.c \
	purgeable_malloc.c vm.c resolver/resolver.c

SLF = /System/Library/Frameworks
SYSPRIVHDR = System.framework/Versions/B/PrivateHeaders
CFLAGS = --sysroot=${RAVYN_SDKROOT} -w -momit-leaf-frame-pointer \
	-mmacos-version-min=${MACOS_VERSION_MIN} -DPRIVATE \
	-DDISPATCH_USE_DTRACE=0 -DOSATOMIC_DEPRECATED=0 \
	-DOSATOMIC_USE_INLINED=0 -DOS_UNFAIR_LOCK_INLINE=0 \
	-D_FORTIFY_SOURCE=0 -DNDEBUG -DDARWINTEST -DMALLOC_DTRACE \
	-DOS_VARIANT_NOTRESOLVED -DOS_VARIANT_RESOLVED \
	-Wno-int-conversion -Wno-implicit-int \
	-Wno-implicit-function-declaration \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${.CURDIR:H}/libsystem_kernel/mig_hdr/include \
	-I${.CURDIR:H}/private -I${.CURDIR}/internal \
	-I${.CURDIR}/resolver -I${RAVYN_SDKROOT}${SLF}/${SYSPRIVHDR} \
	-I${ROOT_BINARY_DIR}/${CpuArch}

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-shared -dylib -umbrella System \
	-Wl,-install_name,/usr/lib/system/libsystem_malloc.dylib \
	-Wl,-current_version,283 -Wl,-compatibility_version,1.0.0 \
	-Wl,-search_paths_first -L${RAVYN_SDKROOT}/usr/lib/system  \
	-Wl,-force_load,${.OBJDIR}/lib${LIB}.a \
	-lCrashReporterClient -lsystem_platform -Wl,-upward-lsystem_c \
	-ldyld -lsystem_kernel \
	-Wl,-interposable_list,${.CURDIR}/xcodeconfig/interposable.list

all: lib${LIB}.dylib
lib${LIB}.dylib: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -o ${.TARGET} ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system \
		${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
