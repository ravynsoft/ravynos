# $FreeBSD$

.PATH: ${SRCTOP}/sys/kern

CFLAGS+=	-I${SRCTOP}/sys/sys
KMOD=	macho
SRCS=	imgact_macho.c \
	vnode_if.h

.include <bsd.kmod.mk>
