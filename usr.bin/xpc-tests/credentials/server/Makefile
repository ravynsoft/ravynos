.include <src.opts.mk>
PROG= xpc-credentials-server-test
SRCS= xpc-credentials-server.c
BINDIR?= /usr/bin

NO_WERROR= yes
MK_MAN=no
CFLAGS+= -g -D__APPLE__ -fblocks
CFLAGS+= -I${.CURDIR}/../../../../include/apple -I${.CURDIR}/../../../../include
CFLAGS+= -I${.CURDIR}/../../../../sys
LIBADD+=  System bsm notify nv xpc sbuf

.include <bsd.prog.mk>
