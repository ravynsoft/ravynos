.PATH: ${.CURDIR}

PROG=	seatd-launch
BINDIR= ${OBJTOP}/CoreServices/WindowServer/Resources
MAN=	
SRCS=	seatd-launch.c 
PACKAGE=WindowServer
WARNS?=	2
CFLAGS+= -I${.CURDIR}/../include -Uunix

make-obj-dirs:
	mkdir -p ${MAKEOBJDIR}

.include <bsd.prog.mk>

beforebuild: make-obj-dirs obj
