.include <bsd.own.mk>

PROG= launchproxy
MAN=

BASE_INCLUDE= -I${.CURDIR}/../../apsl/include
BASE_INCLUDE+=  -I${.CURDIR}/../../include -I${.CURDIR}/../../sys -I${.CURDIR}

DEBUG_FLAGS=-g -O0

CFLAGS+= ${BASE_INCLUDE}
CFLAGS+= -I${.CURDIR}/../../lib/liblaunch
CFLAGS+= -I${.CURDIR}/../../include/mach
LDFLAGS+= -pthread -llaunch -lmach -ldispatch -losxsupport -lBlocksRuntime
SRCS= launchproxy.c

.include <bsd.prog.mk>
