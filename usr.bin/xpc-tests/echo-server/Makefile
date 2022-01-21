.include <src.opts.mk>
PROG= xpc-echo-server
SRCS= xpc-echo-server.c
BINDIR?= /usr/bin

MK_WERROR= no
MK_MAN=no
CFLAGS+= -g -D__APPLE__ -fblocks
CFLAGS+= -I${.CURDIR}/../../../include/apple -I${.CURDIR}/../../../include
CFLAGS+= -I${.CURDIR}/../../../sys
LIBADD+= mach BlocksRuntime osxsupport launch dispatch bsm notify nv xpc pthread sbuf

.include <bsd.prog.mk>
