.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vPre1050
SRCS = gen/FreeBSD/daemon.c

CFLAGS += -U__DARWIN_VERS_1050 -D__DARWIN_VERS_1050=0 -DPRIVATE \
	-DBUILDING_VARIANT -DVARIANT_PRE1050 -I${.CURDIR:H}/fbsdcompat \
	-I${.CURDIR:H}/include -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${.CURDIR:H}/../private \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h

.include <bsd.lib.mk>

