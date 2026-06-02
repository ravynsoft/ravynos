.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = FortifySource
SRCS = secure/strlcat_chk.c \
	secure/chk_fail.c \
	secure/memset_chk.c \
	secure/strlcpy_chk.c \
	secure/strncat_chk.c \
	secure/strncpy_chk.c \
	secure/memmove_chk.c \
	secure/strcpy_chk.c \
	secure/memccpy_chk.c \
	secure/snprintf_chk.c \
	secure/strcat_chk.c \
	secure/vsprintf_chk.c \
	secure/stpcpy_chk.c \
	secure/sprintf_chk.c \
	secure/memcpy_chk.c \
	secure/stpncpy_chk.c \
	secure/vsnprintf_chk.c

CFLAGS += -DHAVE_STDINT_H -DHAVE_CONFIG_H -D_FORTIFY_SOURCE=1 \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd -I${.CURDIR:H}/../private

.include <bsd.lib.mk>
