PROG= launchd

.PATH: .
.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/support
.PATH: ${.CURDIR}/uuid
.PATH: ${.CURDIR}/init


BASE_INCLUDE=  -I. -I${.CURDIR} -I${.CURDIR}/init -I${.CURDIR}/../../include/apple
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys -I${MACHINE_INCLUDES}
DEFINES= -D__APPLE__ -fblocks -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
MIG_FLAGS= ${BASE_INCLUDE} 	${DEFINES}
#override default CFLAGS
CFLAGS= ${MIG_FLAGS}  -D__MigTypeCheck
CFLAGS+= -I${.CURDIR}/../../contrib/openbsm -I${.CURDIR}/../../lib/liblaunch
CFLAGS+= -DHAVE_INTTYPES_H

DEBUG_FLAGS+= -g -O0

LDFLAGS+= -static
NO_WERROR=

helper.h launchd_helperServer.h: ${.CURDIR}/helper.defs
	mig ${MIG_FLAGS} -user /dev/null -server /dev/null -header helper.h \
	-sheader launchd_helperServer.h ${.CURDIR}/helper.defs

jobServer.h jobServer.c job.h: ${.CURDIR}/job.defs helper.h
	mig ${MIG_FLAGS} -sheader jobServer.h ${.CURDIR}/job.defs
job_reply.h job_replyUser.c job_replyServer.c job_replyServer.h: ${.CURDIR}/job_reply.defs
	mig ${MIG_FLAGS} -header job_reply.h -sheader job_replyServer.h ${.CURDIR}/job_reply.defs
${.CURDIR}/job_forward.h job_forwardUser.c job_forwardServer.h: ${.CURDIR}/job_forward.defs
	mig ${MIG_FLAGS} -header job_forward.h -sheader job_forwardServer.h ${.CURDIR}/job_forward.defs
notify.h notifyServer.c notifyServer.h: ${.CURDIR}/notify.defs
	mig ${MIG_FLAGS} -header notify.h -sheader notifyServer.h ${.CURDIR}/notify.defs
mach_excServer.h mach_excServer.c: ${.CURDIR}/mach_exc.defs
	mig ${MIG_FLAGS} -sheader mach_excServer.h ${.CURDIR}/mach_exc.defs
internal.h internalServer.h internalServer.c internalUser.c: ${.CURDIR}/internal.defs
	mig ${MIG_FLAGS} -header internal.h -sheader internalServer.h ${.CURDIR}/internal.defs



SRCS= jobServer.c mach_excServer.c internalServer.c
SRCS+= job_forwardUser.c  job_replyUser.c job_replyServer.c notifyServer.c
SRCS+= internalUser.c
SRCS+= launchd.c core.c kill2.c ktrace.c ipc.c log.c runtime.c
SRCS+= init.c
LIBADD+= auditd bsm util dispatch mach
LIBADD+=BlocksRuntime launch osxsupport xpc nv sbuf pthread


MAN=	launchd.conf.5 launchd.plist.5 launchd.8

launchd.1:
	MAN=

CLEANFILES+= *Server.c *User.c job.h helper.h job_reply.h job_forward.h notify.h \
	internal.h mach_exc.h *Server.h *~ cscope.*
.include <bsd.prog.mk>
