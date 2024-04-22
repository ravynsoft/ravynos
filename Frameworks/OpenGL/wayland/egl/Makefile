.include <src.opts.mk>

LIB=		wayland-egl
SHLIB_MAJOR=    0
SRCS=		wayland-egl-abi-check.c wayland-egl.c 
INCS=		wayland-egl-backend.h wayland-egl-core.h wayland-egl.h

CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR} \
	 -I${MAKEOBJDIR}/../src -I${.CURDIR}/../src
CFLAGS+= -Wno-error=unused-parameter -Wno-error=cast-qual \
         -Wno-error=cast-align
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
