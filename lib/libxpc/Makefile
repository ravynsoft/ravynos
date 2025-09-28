.include <bsd.init.mk>

LIB= xpc
MK_WERROR= no

.PATH: ${.CURDIR}/xpc

BASE_INCLUDE= -I${.CURDIR}/../../include/apple -I${.CURDIR}/../libdispatch
BASE_INCLUDE+= -I${.CURDIR}/../libnv
BASE_INCLUDE+= -I${.CURDIR} -I${.CURDIR}/../../sbin/launchd -I.
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys
CFLAGS+= ${BASE_INCLUDE} -fblocks -g -O2
LDFLAGS+= -lnv

SRCS=	xpc_dictionary.c \
	xpc_misc.c \
	xpc_type.c \
	xpc_array.c \
	xpc_connection.c \
	subr_nvlist.c \
	subr_nvpair.c

INCSDIR= ${INCLUDEDIR}/xpc
INCS=	activity.h \
	base.h \
	connection.h \
	debug.h \
	endpoint.h \
	launchd.h \
	xpc.h

includes: mkincdir
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/xpc/$h ${DESTDIR}${INCSDIR}
.endfor

beforeinstall: mkincdir
mkincdir: .PHONY
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}

.include <bsd.lib.mk>
