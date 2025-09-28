.include <bsd.init.mk>

LIB=	launch

.PATH: .
.PATH: ../../sbin/launchd

BASE_INCLUDE= -I${.CURDIR}/../../include/apple -I${MACHINE_INCLUDES} -I${.CURDIR}/../libxpc/
BASE_INCLUDE+= -I${.CURDIR} -I${.CURDIR}/../../sbin/launchd -I../../sbin/launchd -I${.CURDIR}
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys
BASE_INCLUDE+= -I${.OBJDIR}

DEFINES= -D__APPLE__ -fblocks -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
MIG_FLAGS= ${BASE_INCLUDE} 	${DEFINES}
CFLAGS+= ${MIG_FLAGS} -D__MigTypeCheck -g -O0
CFLAGS+= -I${.CURDIR}/../../contrib/openbsm

helper.h launchd_helperUser.c launchd_helperServer.c helperServer.h: helper.defs
	mig ${MIG_FLAGS} -header helper.h -sheader helperServer.h ${.CURDIR}/helper.defs

job.h jobUser.c jobServer.h: job.defs
	mig ${MIG_FLAGS} -sheader jobServer.h ${.CURDIR}/job.defs

SRCS= 	jobUser.c \
	launchd_helperUser.c \
	launchd_helperServer.c \
	libbootstrap.c \
	liblaunch.c \
	libvproc.c
INCS=	launch.h

CLEANFILES+= *User.c *Server.c job.h helper.h *Server.h launchd_helper.h *~

.include <bsd.lib.mk>
