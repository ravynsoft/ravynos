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

CFLAGS+= -I${.CURDIR}

.include <bsd.prog.mk>
