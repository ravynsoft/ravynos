.include <bsd.own.mk>

PROG= launchctl
MAN= launchctl.1

BASE_INCLUDE= -I${.CURDIR}/../../include/apple

CFLAGS+= ${BASE_INCLUDE} -fblocks
CFLAGS+= -I${.CURDIR}/../../lib/liblaunch
CFLAGS+= -I${.CURDIR}/../../include/mach
LDFLAGS+= -llaunch -lmach -ldispatch
LDFLAGS+= -losxsupport -lBlocksRuntime -pthread
DPADD+= ${LIBJANSSON}
LDFLAGS+= -lutil
LDFLAGS+= -ljansson
SRCS= launchctl.c

.include <bsd.prog.mk>
