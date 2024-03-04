.include <src.opts.mk>

LIB=		wayland
SHLIB_MAJOR=    0
SRCS=		connection.c \
		event-loop.c \
                wayland-client.c \
                wayland-os.c \
                wayland-server.c \
                wayland-shm.c \
                wayland-util.c
INCS=		wayland-client-core.h \
		wayland-client.h \
                wayland-os.h \
                wayland-private.h \
                wayland-server-core.h \
                wayland-server-private.h \
                wayland-server.h \
                wayland-util.h

CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR}
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

wayland-version.h: wayland-version.h.in
	sed -e "s/@WAYLAND_VERSION_MAJOR@/${MAJOR}/" \
            -e "s/@WAYLAND_VERSION_MINOR@/${MINOR}/" \
            -e "s/@WAYLAND_VERSION_MICRO@/${MICRO}/" \
            ${.CURDIR}/wayland-version.h.in > wayland-version.h

.include <bsd.lib.mk>
