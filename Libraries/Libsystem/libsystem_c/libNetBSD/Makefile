.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = NetBSD
SRCS = gen/NetBSD/utmpx.c \
	stdlib/NetBSD/strfmon.c \
	string/NetBSD/memset_s.c \
	gen/NetBSD/rb.c

CFLAGS += -DHAVE_STDINT_H \
	-I${.CURDIR:H}/include -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/nbsdcompat -I${.CURDIR:H}/../private \
	-I${SDK_SOURCE_DIR}/usr/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-include${.CURDIR:H}/nbsdcompat/_nbsd_compat_.h

.include <bsd.lib.mk>

