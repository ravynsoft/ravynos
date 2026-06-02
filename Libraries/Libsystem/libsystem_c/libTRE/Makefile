.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = TRE
SRCS = regex/TRE/lib/regcomp.c \
	regex/TRE/lib/regexec.c \
	regex/TRE/lib/tre-ast.c \
	regex/TRE/lib/tre-compile.c \
	regex/TRE/lib/tre-match-backtrack.c \
	regex/TRE/lib/tre-match-parallel.c \
	regex/TRE/lib/tre-mem.c \
	regex/TRE/lib/tre-parse.c \
	regex/TRE/lib/tre-stack.c

CFLAGS += -DHAVE_STDINT_H -DHAVE_CONFIG_H \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/regex/TRE -I${.CURDIR:H}/regex/FreeBSD \
	-I${.CURDIR:H}/locale/FreeBSD -I${.CURDIR:H}/stdtime/FreeBSD \
	-I${.CURDIR:H}/../private -I${SDK_SOURCE_DIR}/usr/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern

.include <bsd.lib.mk>
