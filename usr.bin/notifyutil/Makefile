.include <src.opts.mk>
PROG= notifyutil

.PATH: .
.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/support
.PATH: ${.CURDIR}/uuid
.PATH: ${.CURDIR}/init


MK_MAN= no

BASE_INCLUDE=  -I. -I${.CURDIR} -I${.CURDIR}/init -I${.CURDIR}/../../apsl/include
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys -I${.CURDIR}/../../lib/libnotify
DEFINES= -D__APPLE__ -fblocks -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
MIG_FLAGS= ${BASE_INCLUDE} 	${DEFINES}
#override default CFLAGS
CFLAGS= ${MIG_FLAGS}  -D__MigTypeCheck
CFLAGS+= -I${.CURDIR}/../../contrib/openbsm -I${.CURDIR}/../../lib/liblaunch
CFLAGS+= -DHAVE_INTTYPES_H

DEBUG_FLAGS+= -g -O0
CFLAGS+= ${DEBUG_FLAGS}

LDFLAGS+= -lBlocksRuntime -lbsm -lauditd
LDFLAGS+= -pthread

notify_ipc.h notify_ipcServer.c: ${.CURDIR}/../../lib/libnotify/notify_ipc.defs
	mig ${MIG_FLAGS} ${.CURDIR}/../../lib/libnotify/notify_ipc.defs

notify_proc: notify_ipcServer.c

LDFLAGS+= -lutil -lauditd -lbsm -ldispatch -lmach -lBlocksRuntime -llaunch -losxsupport -lxpc -lnv -lsbuf -lnotify
SRCS=notifyutil.c


CLEANFILES+= *Server.c *User.c job.h helper.h job_reply.h job_forward.h notify.h \
	internal.h mach_exc.h *Server.h *~ cscope.*
.include <bsd.prog.mk>
