.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vDarwinExtsn
SRCS = gen/FreeBSD/popen.c \
	stdio/FreeBSD/fdopen.c \
	stdio/FreeBSD/fopen.c \
	stdlib/FreeBSD/realpath.c \
	sys/getgroups.c

CFLAGS += -DBUILDING_VARIANT -DVARIANT_DARWINEXTSN \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${.CURDIR:H}/fbsdcompat \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/regex/TRE -I${.CURDIR:H}/regex/FreeBSD \
	-I${.CURDIR:H}/locale/FreeBSD -I${.CURDIR:H}/stdtime/FreeBSD \
	-I${.CURDIR:H}/../private -I${SDK_SOURCE_DIR}/usr/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.popen.o = -DLIBC_ALIAS_POPEN
CFLAGS.fdopen.o = -DLIBC_ALIAS_FDOPEN
CFLAGS.fopen.o = -DLIBC_ALIAS_FOPEN

.include <bsd.lib.mk>
