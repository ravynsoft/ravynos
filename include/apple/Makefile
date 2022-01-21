# $FreeBSD$

.include <bsd.own.mk>

NO_OBJ=
INCS=	spawn.h \
	spawn_private.h \
	libproc.h \
	libproc_internal.h
INCSDIR=${INCLUDEDIR}/apple
SUBDIR=uuid sys

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

.include <bsd.prog.mk>
