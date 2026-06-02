.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vDarwinExtsnCancelable
SRCS = gen/FreeBSD/popen.c \
	stdio/FreeBSD/fdopen.c \
	stdio/FreeBSD/fopen.c \
	stdlib/FreeBSD/realpath.c \
	sys/getgroups.c

CFLAGS += -DBUILDING_VARIANT -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN \
	-DPRIVATE -I${.CURDIR:H}/fbsdcompat -I${.CURDIR:H}/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd -I${.CURDIR:H}/../private \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.popen.o = -DLIBC_ALIAS_POPEN
CFLAGS.fdopen.o = -DLIBC_ALIAS_FDOPEN
CFLAGS.fopen.o = -DLIBC_ALIAS_FOPEN

.include <bsd.lib.mk>

