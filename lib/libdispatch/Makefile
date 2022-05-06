.include <bsd.init.mk>

LIB= dispatch
MK_WERROR= no

.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/dispatch
.PATH: ${.CURDIR}/private
.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/resolver

CFLAGS+=	-I${.CURDIR}/../../include/apple -I${.CURDIR}/os -I${.CURDIR}/private
CFLAGS+=	-I${.CURDIR} -I${MACHINE_INCLUDES} -I${.CURDIR}/../../include -I${.CURDIR}/../../sys 
CFLAGS+=	-I. -I${.CURDIR}/include -fblocks -fPIC
CFLAGS+=	-I${SRCTOP}/include
CFLAGS+=	-D__APPLE__ -DDISPATCH_USE_DTRACE=0
CFLAGS+=	-DDISPATCH_USE_SIMPLE_ASL=0 -DUSE_OBJC=0 -D__BLOCKS__=1
CFLAGS+=	-DDISPATCH_DEBUG=1

protocolServer.c protocolUser.c protocol.h protocolServer.h: ${.CURDIR}/src/protocol.defs 
	mig ${CFLAGS}  -user protocolUser.c \
	-server protocolServer.c -header protocol.h \
	-sheader protocolServer.h ${.CURDIR}/src/protocol.defs

SRCS =  protocolUser.c \
	protocolServer.c \
	allocator.c \
	apply.c	\
	benchmark.c \
	data.c \
	init.c \
	introspection.c	\
	io.c \
	object.c \
	once.c \
	queue.c \
	semaphore.c \
	source.c \
	time.c \
	transform.c \
	voucher.c \
	resolver.c 
#	object.m

INCSDIR= ${INCLUDEDIR}/dispatch
INCS=	base.h \
	block.h \
	data.h \
	dispatch.h \
	group.h \
	introspection.h \
	io.h \
	object.h \
	once.h \
	private.h \
	queue.h \
	semaphore.h \
	source.h \
	time.h

INCS+=	benchmark.h \
	data_private.h \
	introspection_private.h \
	io_private.h \
	layout_private.h \
	mach_private.h \
	private.h \
	queue_private.h \
	source_private.h \
	voucher_activity_private.h \
	voucher_private.h

CLEANFILES+= protocol.h protocolServer.c protocolServer.h protocolUser.c

includes: mkincdir installincludes
beforeinstall: mkincdir
mkincdir: .PHONY
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}

.include <rvn.common.mk>
.include <bsd.lib.mk>
