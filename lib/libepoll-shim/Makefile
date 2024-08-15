PACKAGE=lib${LIB}
.PATH: ${.CURDIR}/src

.include <src.opts.mk>

LIB=		epoll-shim
SHLIB_MAJOR=    0

SRCS=	        include/sys/epoll.h \
                .WAIT \
                compat_kqueue1.c \
                compat_itimerspec.c \
                kqueue_event.c \
                epoll_shim_ctx.c \
                timerfd.c \
                signalfd_ctx.c \
                eventfd_ctx.c \
                rwlock.c \
                compat_sigops.c \
                epollfd_ctx.c \
                signalfd.c \
                timerfd_ctx.c \
                compat_socketpair.c \
                epoll_shim_interpose.c \
                eventfd.c \
                wrap.c \
                pollrdhup.c \
                compat_pipe2.c \
                epoll.c \
                compat_ppoll.c \
                timespec_util.c \
                compat_sem.c \
                compat_socket.c 


INCSDIR=        ${INCLUDEDIR}/${PACKAGE}/sys
INCS=	        include/sys/epoll.h \
                include/sys/eventfd.h \
                include/sys/timerfd.h \
                include/sys/signalfd.h

CFLAGS+= -Iinclude -I${.CURDIR}/src
WARNS?=	1
MK_WERROR= no
WERROR= no

include/epoll-shim::
	cp -Rv ${.CURDIR}/include/epoll-shim ${DESTDIR}${INCLUDEDIR}/${PACKAGE}/
 
include/sys/epoll.h::
	cp -R ${.CURDIR}/include .
	mkdir -p ${OBJTOP}/tmp/${INCSDIR}
	mkdir -p ${MAKEOBJDIR}/include/sys
	mkdir -p ${INCSDIR}/${PACKAGE}/sys
	sed -e 's/@POLLRDHUP_VALUE@/0x4000/' \
		<${.CURDIR}/include/sys/epoll.h \
		>${MAKEOBJDIR}/include/sys/epoll.h

makedirs:
	mkdir -p ${OBJTOP}/release/dist/base/${INCSDIR}


.include <bsd.lib.mk>

includes: include/epoll-shim
distribute: makedirs
