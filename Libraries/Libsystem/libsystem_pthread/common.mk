MK_WERROR = no
SRCS = \
	pthread.c pthread_asm.s pthread_atfork.c pthread_cancelable.c \
	pthread_cond.c pthread_cwd.c pthread_dependency.c pthread_mutex.c \
	pthread_rwlock.c pthread_tsd.c qos.c resolver/resolver.c \
	variants/pthread_cancelable_cancel.c \
	variants/pthread_cancelable_legacy.c variants/pthread_cond_legacy.c \
	variants/pthread_mutex_legacy.c variants/pthread_rwlock_legacy.c

SLF = /System/Library/Frameworks
SYSPRIVHDR = System.framework/Versions/B/PrivateHeaders
LIBSYSBIN = ${ROOT_BINARY_DIR}/Libraries/Libsystem
CFLAGS = --sysroot=${RAVYN_SDKROOT} -momit-leaf-frame-pointer \
        -mmacos-version-min=${MACOS_VERSION_MIN} \
	-fno-stack-protector -fno-stack-check -fno-builtin \
	-Wno-implicit-function-declaration -Wno-nullability-completeness \
	-Wno-int-conversion -Wno-implicit-int -Wno-incompatible-sysroot \
	-Wno-expansion-to-defined -DPRIVATE -DDISPATCH_USE_DTRACE=0 \
	-D__LIBC__ -D__DARWIN_UNIX03 -D__DARWIN_64_BIT_INO_T \
	-D__DARWIN_NON_CANCELABLE -D__DARWIN_VERS_1050 -D_FORTIFY_SOURCE=0 \
	-D__PTHREAD_BUILDING_PTHREAD__ -D__PTHREAD_EXPOSE_INTERNALS__ \
	-DOS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${LIBSYSBIN}/libsystem_kernel/mig_hdr/include \
	-I${LIBSYSBIN}/libsystem_kernel/mig_hdr/local/include \
	-I${.CURDIR} -I${.CURDIR}/private -I${.CURDIR}/../private \
	-I${.CURDIR}/resolver -I${.CURDIR}/../libsystem_c/darwin \
	-I${RAVYN_SDKROOT}/usr/local/include/pthread \
	-I${ROOT_BINARY_DIR}/${CpuArch} -I${RAVYN_SDKROOT}${SLF}/${SYSPRIVHDR} \
	-I${ROOT_SOURCE_DIR}/Kernel/Extensions/pthread \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
