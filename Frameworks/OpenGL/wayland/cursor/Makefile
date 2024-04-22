.include <src.opts.mk>

LIB=		wayland-cursor
SHLIB_MAJOR=    0
PROTOCOLDIR=	${.CURDIR}/../protocol
SCANNER=	${MAKEOBJDIR}/../scanner/wayland-scanner

SRCS= 	convert_font.c \
        os-compatibility.c \
        wayland-cursor.c \
        xcursor.c

INCS=	cursor-data.h \
	os-compatibility.h \
        wayland-cursor.h \
        xcursor.h

CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR} \
	 -I${MAKEOBJDIR}/../src -I${.CURDIR}/../src
CFLAGS+= -Wno-error=incompatible-pointer-types-discards-qualifiers \
         -Wno-error=gnu-pointer-arith \
         -Wno-error=strict-prototypes \
         -Wno-error=missing-prototypes \
         -Wno-error=cast-qual -Wno-error=cast-align
CFLAGS+= -DHAVE_ACCEPT4 \
	-DHAVE_BROKEN_MSG_CMSG_CLOEXEC=0 \
	-DHAVE_MEMFD_CREATE \
	-DHAVE_MKOSTEMP \
	-DHAVE_POSIX_FALLOCATE \
	-DHAVE_STRNDUP \
	-DHAVE_SYS_PROCCTL_H \
	-DHAVE_SYS_UCRED_H \
	-DHAVE_XUCRED_CR_PID=1 \
	-DPACKAGE="${PACKAGE}" \
	-DPACKAGE_VERSION="${MAJOR}.${MINOR}.${MICRO}"

WARNS?=	2

.include <bsd.lib.mk>
