SLF = /System/Library/Frameworks
SYSPRIVHDR = System.framework/Versions/B/PrivateHeaders
XNU = ${ROOT_SOURCE_DIR}/Kernel/xnu
MACH_DEFS != echo ${XNU}/osfmk/mach/*.defs

.PATH: ${XNU}/libsyscall
.PATH: ${XNU}/libsyscall/wrappers
.PATH: ${XNU}/libsyscall/custom
.PATH: ${XNU}/libsyscall/os
.PATH: ${XNU}/libsyscall/mach

MK_WERROR = no

LIB = system_kernel

CFLAGS.os_packet.o = -fno-builtin
CFLAGS.os_channel.o = -fno-builtin
CFLAGS.os_nexus.o = -fno-builtin
# seems to be a version at Kernel/xnu/bsd/netinet/cpu_in_cksum_gen.c
CFLAGS.cpu_copy_in_cksum_gen.o = -fno-builtin
CFLAGS.cpu_in_cksum_gen.o = -fno-builtin
CFLAGS.os_channel_event.o = -fno-builtin

CFLAGS = \
	--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-implicit-int -Wno-implicit-function-declaration \
	-Wno-int-conversion -Wno-incompatible-sysroot \
	-Wno-nullability-completeness -no-cpp-precomp -nostdlib \
	-fdollars-in-identifiers -fno-common -fno-stack-protector \
	-fno-stack-check -momit-leaf-frame-pointer \
	-D__DARWIN_VERS_1050 -DLIBSYSCALL_INTERFACE -DSYSCALL_PRE1050 \
	-DNO_SYSCALL_LEGACY -DCF_OPEN_SOURCE -DCF_EXCLUDE_CSTD_HEADERS \
	-DDEBUG -D_FORTIFY_SOURCE=0 -DDARWIN_C_SOURCE=DARWIN_C_FULL \
	-DPRIVATE -DTARGET_OS_BRIDGE=0 -D__DARWIN_UNIX03 \
	-I${XNU}/bsd -I${XNU}/libsyscall/mach -I${.OBJDIR} \
	-I${XNU}/libsyscall/os \
	-I${XNU}/libsyscall/wrappers/spawn \
	-I${ROOT_SOURCE_DIR}/Frameworks \
	-I${XNU}/libsyscall/wrappers \
	-I${XNU}/libsyscall/wrappers/string \
	-I${XNU}/libsyscall/wrappers/libproc \
	-I${XNU}/libsyscall \
	-I${XNU}/libsyscall/mach \
	-I${ROOT_BINARY_DIR}/${CpuArch} \
	-I${RAVYN_SDKROOT}/usr/local/include \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}${SLF}/${SYSPRIVHDR} \
	-I${.OBJDIR}/internal_hdr/include \
	-I${.OBJDIR}/internal_hdr/include/mach \
	-I${.OBJDIR}/${CpuArch} \
	-I${.OBJDIR}/mig_hdr/include/mach \
	-I${.OBJDIR}/mig_hdr/include \
	-I${.OBJDIR}/mig_hdr/include/servers \
	-I${XNU}/osfmk ${CFLAGS.${.TARGET:T}}

ACFLAGS = -x assembler-with-cpp ${CFLAGS}

# legacy syscalls
SRCS = \
	legacy/accept.c \
	legacy/bind.c \
	legacy/getattrlist.c \
	legacy/getpeername.c \
	legacy/getsockname.c \
	legacy/lchown.c \
	legacy/listen.c \
	legacy/recvfrom.c \
	legacy/recvmsg.c \
	legacy/sendmsg.c \
	legacy/sendto.c \
	legacy/setattrlist.c \
	legacy/socketpair.c \
	legacy/connect.c \
	legacy/mprotect.c \
	legacy/msync.c \
	legacy/open.c \
	legacy/select-pre1050.c \
	legacy/select.c \
	legacy/getaudit.c

# unix03 syscalls
SRCS += \
	unix03/chmod.c \
	unix03/fchmod.c \
	unix03/getrlimit.c \
	unix03/mmap.c \
	unix03/munmap.c \
	unix03/setrlimit.c

# cancelable syscalls
SRCS += \
	cancelable/pselect-darwinext-cancel.c \
	cancelable/pselect-darwinext.c \
	cancelable/fcntl.c \
	cancelable/fcntl-cancel.c \
	cancelable/select.c \
	cancelable/select-cancel.c \
	cancelable/sigsuspend.c \
	cancelable/sigsuspend-cancel.c

# custom syscall wrappers
SRCS += \
	custom/errno.c

# These Skywalk sources are all empty in the public xnu
SRCS += \
	skywalk/os_packet.c \
	skywalk/cpu_in_cksum.s \
	skywalk/os_channel.c \
	skywalk/os_nexus.c \
	skywalk/os_channel_event.c \
	skywalk/cpu_copy_in_cksum.s \
	skywalk/cpu_copy_in_cksum_gen.c \
	skywalk/cpu_in_cksum_gen.c

# General wrappers
SRCS += \
	mach_approximate_time.c \
	mach_continuous_time.c \
	mach_absolute_time.s \
	mach_timebase_info.c \
	mach_bridge_remote_time.c \
	mach_get_times.c \
	mach_boottime.c \
	libproc/libproc.c \
	libproc/proc_listpidspath.c \
	proc.c \
	spawn/posix_spawn.c \
	terminate_with_reason.c \
	kill.c \
	ioctl.c \
	rename.c \
	clonefile.c \
	unlinkat.c \
	renameat.c \
	renamex.c \
	unlink.c \
	rmdir.c \
	carbon_delete.c \
	utimensat.c \
	setpriority.c \
	guarded_open_dprotected_np.c \
	quota_obsolete.c \
	getiopolicy_np.c \
	gethostuuid.c \
	thread_register_state.c \
	stackshot.c \
	sfi.c \
	posix_sem_obsolete.c \
	pid_shutdown_networking.c \
	coalition.c \
	fs_snapshot.c \
	work_interval.c \
	remove-counter.c \
	open_dprotected_np.c \
	varargs_wrappers.s \
	persona.c \
	guarded_open_np.c \
	kdebug_trace.c \
	reboot.c \
	csr.c \
	init_cpu_capabilities.c \
	__commpage_gettimeofday.c \
	__get_cpu_capabilities.s \
	_libkernel_init.c \
	string/strlen.c \
	string/index.c \
	string/strcmp.c \
	string/memset.c \
	string/memcpy.c \
	string/strsep.c \
	string/strcpy.c \
	string/strlcpy.c \
	alloc_once.c \
	log_data.c

# mach traps
SRCS += \
	port_descriptions.c \
	clock_sleep.c \
	error_codes.c \
	exc_catcher_state_identity.c \
	exc_catcher_state.c \
	exc_catcher.c \
	fprintf_stderr.c \
	host.c \
	mach_error_string.c \
	mach_error.c \
	mach_init.c \
	mach_msg.c \
	mach_traps.s \
	mig_allocate.c \
	mig_deallocate.c \
	mig_reply_setup.c \
	mig_reply_port.c \
	mig_strncpy.c \
	ms_thread_switch.c \
	panic.c \
	port_obj.c \
	semaphore.c \
	slot_name.c \
	string.c \
	mach_legacy.c \
	mach_right.c \
	thread_act.c \
	mach_port.c \
	mach_vm.c
#OBJS = ${SRCS:S/.c$/.o/:S/.s$/.o/}

LDFLAGS = \
	-umbrella System -mmacos-version-min=${MACOS_VERSION_MIN} \
	-L${RAVYN_SDKROOT}/usr/lib/system \
	-Wl,-install_name,/usr/lib/system/libsystem_kernel.dylib \
	-Wl,-current_version,6153.0.168 -Wl,-compatibility_version,1.0.0

MIG_FLAGS = -novouchers -arch ${CpuArch} -cc ${MIGCC} \
	-I${RAVYN_SDKROOT}/usr/include -I${RAVYN_SDKROOT}/usr/local/include \
	-mmacos-version-min=${MACOS_VERSION_MIN} -fno-stack-check \
	-fno-stack-protector

all: lib${LIB}.dylib
lib${LIB}.dylib: ${.OBJDIR}/lib${LIB}.dylib

copy_defs: ${MACH_DEFS}
	mkdir -p ${RAVYN_SDKROOT}${SLF}/${SYSPRIVHDR}/mach
	cp -f ${MACH_DEFS} ${RAVYN_SDKROOT}${SLF}/${SYSPRIVHDR}/mach/

# Generate syscall stubs - 24614EF311E7C98600E78584 libsyscalls.a
${.OBJDIR}/libsyscalls.a: ${XNU}/bsd/kern/syscalls.master copy_defs
	mkdir -p ${.OBJDIR}/sys
	env SDKROOT=${RAVYN_SDKROOT} BUILT_PRODUCTS_DIR=${.OBJDIR} \
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer \
	PATH=${TOOLS}:${PATH} ARCHS=${CpuArch} SRCROOT=${XNU}/libsyscall \
	OBJROOT=${.OBJDIR} \
	${XNU}/libsyscall/xcodescripts/create-syscalls.pl \
	  ${XNU}/bsd/kern/syscalls.master ${XNU}/libsyscall/custom \
	  ${XNU}/libsyscall/Platforms MacOSX ${.OBJDIR}/sys
	env SDKROOT=${RAVYN_SDKROOT} SDK_INSTALL_HEADERS_ROOT=. \
	BUILT_PRODUCTS_DIR=${.OBJDIR} \
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer PATH=${TOOLS}:${PATH} \
	ARCHS=${CpuArch} SRCROOT=${XNU}/libsyscall OBJROOT=${.OBJDIR} \
	${XNU}/libsyscall/xcodescripts/compile-syscalls.pl \
	  ${.OBJDIR}/sys/stubs.list ${.TARGET}

${.OBJDIR}/libmach.a: ${MACH_DEFS}
	mkdir -p ${.OBJDIR}/obj/mig_hdr/include/mach \
	  ${.OBJDIR}/obj/mig_hdr/include/servers \
	  ${.OBJDIR}/obj/internal_hdr/include/mach \
	  ${.OBJDIR}/${CpuArch}
	SDKROOT=${RAVYN_SDKROOT} PLATFORM_NAME=MacOSX ARCHS=${CpuArch} \
	SRCROOT=${XNU}/libsyscall OBJROOT=${.OBJDIR}/obj \
	BUILT_PRODUCTS_DIR=${.OBJDIR} \
	DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer PATH=${TOOLS}:${PATH} \
	CFLAGS="${CFLAGS}" ${XNU}/libsyscall/xcodescripts/mach_install_mig.sh
	set -e; for f in ${.OBJDIR}/obj/*User.c; \
	do \
	    ${CC} ${CFLAGS} -o $$f.o -c $$f; \
	done
	${AR} rcs ${.TARGET} ${.OBJDIR}/obj/*.o

.for def in thread_act.defs mach_port.defs vm_map.defs mach_vm.defs
${.OBJDIR}/mach/${def:R}_internal.h: ${XNU}/libsyscall/mach/${def}
	mkdir -p ${.OBJDIR}/mach
	${MIG} ${MIG_FLAGS} -header ${.TARGET} ${.ALLSRC}

${.OBJDIR}/lib${LIB}.a: ${.OBJDIR}/mach/${def:R}_internal.h
.endfor

lib${LIB}.a: ${.OBJDIR}/libmach.a ${.OBJDIR}/libsyscalls.a

${.OBJDIR}/_libc_funcptr.o: ${XNU}/libsyscall/wrappers/_libc_funcptr.c
	${CC} -c ${.ALLSRC} -o ${.TARGET} ${CFLAGS}

${.OBJDIR}/lib${LIB}.dylib: ${.OBJDIR}/_libc_funcptr.o lib${LIB}.a
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib/kernel
	${TOOLS}/llvm-libtool-darwin -static -no_warning_for_no_symbols \
	  -o ${.OBJDIR}/libkernel.a ${.OBJDIR}/lib${LIB}.a \
		${.OBJDIR}/libmach.a ${.OBJDIR}/libsyscalls.a
	${CC} -shared -dylib -fuse-ld=${LD} -nostdlib -o ${.TARGET} ${LDFLAGS} \
	  -Wl,-force_load,${.OBJDIR}/libkernel.a \
	  -Wl,-force_load,${.OBJDIR}/_libc_funcptr.o \
	  -Wl,-install_name,/usr/lib/system/libsystem_kernel.dylib \
	  -Wl,-current_version,6153.0.168 \
	  -Wl,-compatibility_version,1.0.0 \
	  -L${RAVYN_SDKROOT}/usr/lib/system -lsystem_c -ldyld
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/
	cp -f ${.OBJDIR}/libkernel.a \
	  ${RAVYN_SDKROOT}/usr/local/lib/kernel/libkernel.a

.include <bsd.lib.mk>
