# $FreeBSD$

LIB=	asl
SHLIB_MAJOR=	1
SHLIBDIR?= /usr/lib
MK_WERROR=no

DEFINES= -D__APPLE__ -fblocks -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
BASE_INCLUDE= -I${.CURDIR}/../../include/apple -I${.CURDIR}/../liblaunch/ -I${.CURDIR}/../libosxsupport/ -I${.CURDIR}/../libdispatch/ -I${.CURDIR}/../libxpc/
BASE_INCLUDE+= -I${.CURDIR} -I${.CURDIR}/../../include -I${MACHINE_INCLUDES} -I${.CURDIR}/../../sys
BASE_INCLUDE+= -I${SRCTOP}/include -I${SRCTOP}/sys
MIG_FLAGS= ${BASE_INCLUDE} ${DEFINES}
CFLAGS+= ${BASE_INCLUDE} ${DEFINES} -fPIC -DPRIVATE -D__MigTypeCheck=1 -DLIBSYSCALL_INTERFACE=1

SRCS=	asl.c asl_ipcUser.c asl_core.c asl_file.c asl_msg.c asl_object.c \
	asl_string.c syslog.c asl_client.c asl_fd.c asl_legacy1.c \
	asl_msg_list.c asl_store.c asl_util.c asl_common.c asl_memory.c

MAN=	asl.3 syslog.3

asl_ipc.h asl_ipcUser.c: asl_ipc.defs 
	mig ${MIG_FLAGS} -sheader asl_ipc.h ${.CURDIR}/asl_ipc.defs

asl.c: asl_ipc.h

INCS=	asl.h

.include <bsd.lib.mk>
