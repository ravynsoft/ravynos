# $FreeBSD$

.include <bsd.own.mk>

NO_OBJ=
INCS=	uuid.h
INCSDIR=${INCLUDEDIR}/apple/uuid

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

