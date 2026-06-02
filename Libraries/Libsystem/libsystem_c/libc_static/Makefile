.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = c_static
SRCS = secure/chk_fail.c \
	secure/memcpy_chk.c \
	secure/memmove_chk.c \
	secure/memset_chk.c \
	secure/snprintf_chk.c \
	secure/sprintf_chk.c \
	secure/stpcpy_chk.c \
	secure/stpncpy_chk.c \
	secure/strcat_chk.c \
	secure/strcpy_chk.c \
	secure/strlcat_chk.c \
	secure/strlcpy_chk.c \
	secure/strncat_chk.c \
	secure/strncpy_chk.c \
	secure/vsnprintf_chk.c \
	secure/vsprintf_chk.c \
	secure/memccpy_chk.c

.if "${CpuArch}" == "x86_64"
SRCS += x86_64/gen/mcount.s
.endif

CFLAGS += -UBUILDING_VARIANT -U__DARWIN_NON_CANCELABLE \
	-DVARIANT_STATIC -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN  \
	-D__DARWIN_NON_CANCELABLE=0 \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${.CURDIR:H}/fbsdcompat -I${.CURDIR:H}/gdtoa \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/../private \
	-I${.CURDIR:H}/../libsystem_info/membership.subproj \
	-I${.CURDIR:H}/../libsystem_info/lookup.subproj \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern

all: libc.a

libc.a: lib${LIB}.a
	${TOOLS}/llvm-libtool-darwin -o ${.OBJDIR}/libc.a -static \
	  ${.OBJDIR}/libc_static.a ${.OBJDIR:H}/libBase/libBase.a \
	  ${.OBJDIR:H}/libNetBSD/libNetBSD.a \
	  ${.OBJDIR:H}/libFortifySource/libFortifySource.a \
	  ${.OBJDIR:H}/libFreeBSD/libFreeBSD.a ${.OBJDIR:H}/libTRE/libTRE.a \
	  ${.OBJDIR:H}/vCancelable/libvCancelable.a \
	  ${.OBJDIR:H}/vDarwinExtsn/libvDarwinExtsn.a
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib/system \
		${RAVYN_SDKROOT}/usr/local/lib/loaderd
	cp -f ${.OBJDIR}/libc.a ${RAVYN_SDKROOT}/usr/local/lib/system/
	ln -sf ../system/libc.a ${RAVYN_SDKROOT}/usr/local/lib/loaderd/libc.a

.include <bsd.lib.mk>
