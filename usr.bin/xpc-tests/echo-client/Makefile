.include <src.opts.mk>
PROG= xpc-echo-client
SRCS= xpc-echo-client.c
BINDIR?= /usr/bin

MK_WERROR= no
MK_MAN=no
CFLAGS+= -g -D__APPLE__ -fblocks
CFLAGS+= -I${.CURDIR}/../../../include/apple -I${.CURDIR}/../../../include
CFLAGS+= -I${.CURDIR}/../../../sys
LIBADD+= mach BlocksRuntime dispatch osxsupport launch bsm notify nv sbuf xpc pthread

.include <bsd.prog.mk>
