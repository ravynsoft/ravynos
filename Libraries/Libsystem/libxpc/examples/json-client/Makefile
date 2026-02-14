.include <src.opts.mk>
PROG= xpc-json-client
SRCS= xpc-json-client.c
BINDIR?= /usr/bin

NO_WERROR= yes
MK_MAN=no
CFLAGS+= -g -D__APPLE__ -fblocks
CFLAGS+= -I${.CURDIR}/../../../include/apple -I${.CURDIR}/../../../include
CFLAGS+= -I${.CURDIR}/../../../sys
LDADD+= -lmach -lBlocksRuntime -ldispatch -losxsupport -llaunch -lbsm -lnv -lsbuf -lxpc -pthread -ljansson

.include <bsd.prog.mk>
