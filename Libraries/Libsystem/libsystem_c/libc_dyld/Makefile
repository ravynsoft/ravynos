.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = c_dyld
SRCS = gen/FreeBSD/arc4random.c \
	gen/FreeBSD/closedir.c \
	gen/dirfd.c \
	gen/FreeBSD/getcwd.c \
	gen/FreeBSD/getpagesize.c \
	gen/nanosleep.c \
	gen/FreeBSD/opendir.c \
	gen/FreeBSD/readdir.c \
	gen/FreeBSD/scandir.c \
	gen/FreeBSD/sysctl.c \
	gen/FreeBSD/sysctlbyname.c \
	gen/FreeBSD/telldir.c \
	gen/FreeBSD/usleep.c \
	stdlib/FreeBSD/exit.c \
	stdlib/FreeBSD/heapsort.c \
	stdlib/FreeBSD/merge.c \
	stdlib/FreeBSD/qsort.c \
	stdlib/FreeBSD/reallocf.c \
	stdlib/FreeBSD/realpath.c \
	string/bcopy.c \
	string/strcat.c \
	string/FreeBSD/strdup.c \
	string/FreeBSD/strrchr.c \
	sys/gettimeofday.c \
	sys/_libc_init.c

	#stdlib/FreeBSD/atexit.c 

CFLAGS += -DPRIVATE -U__DARWIN_NON_CANCELABLE -D__DARWIN_NON_CANCELABLE=0 \
	-DVARIANT_STATIC -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${.CURDIR:H}/fbsdcompat -I${.CURDIR:H}/gdtoa \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/../private \
	-I${.CURDIR:H}/../libsystem_info/membership.subproj \
	-I${.CURDIR:H}/../libsystem_info/lookup.subproj \
	-I${.CURDIR:H}/../libsystem_blocks \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

SLF = /System/Library/Frameworks
CFLAGS.arc4random.o = -include${RAVYN_SDKROOT}/usr/include/os/availability.h \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders

CFLAGS.closedir.o = -DLIBC_ALIAS_CLOSEDIR \
		-include${.CURDIR:H}/gen/__dirent.h
CFLAGS.bcopy.o = -momit-leaf-frame-pointer
CFLAGS.opendir.o = -DLIBC_ALIAS___OPENDIR2 -DLIBC_ALIAS_OPENDIR \
		-include${.CURDIR:H}/gen/__dirent.h
CFLAGS.readdir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.scandir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.telldir.o = -DLIBC_ALIAS__SEEKDIR -DLIBC_ALIAS_TELLDIR \
		-include${.CURDIR:H}/gen/__dirent.h
CFLAGS.usleep.o = -DLIBC_ALIAS_USLEEP
CFLAGS.nanosleep.o = -DLIBC_ALIAS_NANOSLEEP
CFLAGS.realpath.o = -DLIBC_ALIAS_REALPATH
CFLAGS.gettimeofday.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS._libc_init.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders

all: lib${LIB}.a copylib

copylib: lib${LIB}.a
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib/dyld
	cp -f ${.OBJDIR}/libc_dyld.a ${RAVYN_SDKROOT}/usr/local/lib/dyld/libc.a

.include <bsd.lib.mk>

