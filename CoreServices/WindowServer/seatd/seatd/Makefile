.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/../common

PROG=	seatd
BINDIR= ${OBJTOP}/CoreServices/WindowServer/Resources
MAN=	
SRCS=	client.c \
	poller.c \
        seat.c \
        seatd.c \
        server.c \
	connection.c \
        linked_list.c \
        log.c \
	terminal.c \
        evdev.c \
        drm.c \
        wscons.c

PACKAGE=WindowServer
WARNS?=	2
CFLAGS+= -I${.CURDIR}/../include -I${.CURDIR}/../../libevdev/include -Uunix
CFLAGS+= -I${OBJTOP}/tmp/usr/include
LDFLAGS+= -Wl,-R'$$ORIGIN'

make-obj-dirs:
	mkdir -p ${MAKEOBJDIR}

.include <bsd.prog.mk>

beforebuild: make-obj-dirs obj
