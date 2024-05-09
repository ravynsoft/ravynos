MK_AUTO_OBJ=    yes
.include <sys.mk>
.include <src.opts.mk>

.PATH:          ${.CURDIR}
.PATH:          ${MAKEOBJDIR}
LIB=		wayland
SHLIB_MAJOR=    0
SRCS=		connection.c \
		event-loop.c \
                wayland-client.c \
                wayland-os.c \
                wayland-server.c \
                wayland-shm.c \
                wayland-util.c \
                wayland-protocol.c \
		wayland-client-protocol-core.h \
                wayland-server-protocol-core.h \
                wayland-version.h \
                wayland-server-protocol.h \
                wayland-client-protocol.h 

INCS=		wayland-client-protocol.h \
                wayland-os.h \
                wayland-private.h \
                wayland-server-private.h \
                wayland-server-protocol.h \
                wayland-util.h

CFLAGS+= --sysroot=${OBJTOP}/tmp -I${.CURDIR} -I${MAKEOBJDIR} \
	 -I${OBJTOP}/lib/libffi/include \
	 -I${OBJTOP}/tmp/usr/include/libepoll-shim -I${.CURDIR}/include
CFLAGS+= -Wno-error=cast-align -Wno-error=missing-variable-declarations \
         -Wno-error=unused-parameter -Wno-error=cast-qual \
         -Wno-error=thread-safety-analysis -Wno-error=unused-parameter
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
SCANNER=        ${MAKEOBJDIR}/../scanner/wayland-scanner
PROTOCOLDIR=    ${.CURDIR}/../protocol

wayland-version.h: wayland-version.h.in
	sed -e "s/@WAYLAND_VERSION_MAJOR@/${MAJOR}/" \
            -e "s/@WAYLAND_VERSION_MINOR@/${MINOR}/" \
            -e "s/@WAYLAND_VERSION_MICRO@/${MICRO}/" \
            ${.CURDIR}/wayland-version.h.in > wayland-version.h

wayland-protocol.c: ${PROTOCOLDIR}/wayland.xml
	${SCANNER} -s public-code ${.ALLSRC} ${.TARGET}

wayland-server-protocol.h: ${PROTOCOLDIR}/wayland.xml
	${SCANNER} -s server-header ${.ALLSRC} ${.TARGET}

wayland-server-protocol-core.h: ${PROTOCOLDIR}/wayland.xml
	${SCANNER} -s server-header -c ${.ALLSRC} ${.TARGET}

wayland-client-protocol.h: ${PROTOCOLDIR}/wayland.xml
	${SCANNER} -s client-header ${.ALLSRC} ${.TARGET}

wayland-client-protocol-core.h: ${PROTOCOLDIR}/wayland.xml
	${SCANNER} -s client-header -c ${.ALLSRC} ${.TARGET}

.include <bsd.lib.mk>

