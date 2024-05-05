PACKAGE=lib${LIB}
.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/../common
.PATH: ${.CURDIR}/backend

.include <src.opts.mk>

LIB=		seat
SHLIB_MAJOR=    ${MAJOR}

SRCS=   noop.c \
        seatd.c \
        libseat.c \
	connection.c \
	linked_list.c \
	log.c
INCS=

CFLAGS+= -I${.CURDIR}/include -I${.CURDIR}/../include -DLIBSEAT=1 \
         -DSEATD_ENABLED=1 -Uunix
WARNS?=	2

make-obj-dirs:
	mkdir -p ${MAKEOBJDIR}

.include <bsd.lib.mk>

beforebuild: make-obj-dirs obj

