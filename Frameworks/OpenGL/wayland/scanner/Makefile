.PATH:  ${.CURDIR}/../src
PROG=	wayland-scanner
MK_MAN= no
HAS_TESTS=
SRCS=	scanner.c wayland-util.c wayland-version.h wayland.dtd.h
CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR} \
         -Wno-error=cast-align -Wno-error=unused-parameter
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
LDFLAGS= -lbsdxml
WARNS?=		2

wayland.dtd.h::
	${OBJTOP}/tmp/usr/bin/python3 ${.CURDIR}/../src/embed.py \
        ${.CURDIR}/../protocol/wayland.dtd wayland_dtd >${.TARGET}

wayland-version.h: ../src/wayland-version.h.in
	sed -e "s/@WAYLAND_VERSION_MAJOR@/${MAJOR}/" \
            -e "s/@WAYLAND_VERSION_MINOR@/${MINOR}/" \
            -e "s/@WAYLAND_VERSION_MICRO@/${MICRO}/" \
            ${.CURDIR}/../src/wayland-version.h.in > wayland-version.h

_libinstall:
	echo ${PROG} ${.TARGET}

.include <bsd.prog.mk>
