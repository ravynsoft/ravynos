.include <bsd.init.mk>

LIB=	notify
NO_WERROR=yes
NO_WTHREAD_SAFETY=yes

.PATH: .
.PATH: ../../sbin/launchd

BASE_INCLUDE= -I${.CURDIR}/../../include/apple -I${MACHINE_INCLUDES}
BASE_INCLUDE+= -I${.CURDIR} -I. -I${.CURDIR}/../liblaunch -I${.CURDIR}/../libosxsupport
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys -I${.CURDIR}
DEFINES= -D__APPLE__ -fblocks -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
MIG_FLAGS= ${BASE_INCLUDE} 	${DEFINES}
CFLAGS+= ${MIG_FLAGS} -D__MigTypeCheck 

notify_ipc.h notify_ipcUser.c: notify_ipc.defs
	mig ${MIG_FLAGS} ${.CURDIR}/notify_ipc.defs

notify_client.c: notify_ipc.h notify_ipcUser.c

SRCS=	libnotify.c \
	notify_client.c \
	table.c \
	notify_ipcUser.c
INCS=	notify.h

CLEANFILES+= *User.c *Server.c notify_ipc.h

.include <bsd.lib.mk>
