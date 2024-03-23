PACKAGE=lib${LIB}
.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/../common

.include <src.opts.mk>

LIB=		seatd
SHLIB_MAJOR=    ${MAJOR}

SRCS=   noop.c \
        seatd.c \
        libseat.c \
	connection.c \
	linked_list.c \
	log.c
INCS=	libseat.h

CFLAGS+= -I${.CURDIR}/include -DLIBSEAT=1 -DSEATD_ENABLED=1 

WARNS?=	2

.include <bsd.lib.mk>
