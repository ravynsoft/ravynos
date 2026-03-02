# $FreeBSD$

NO_OBJ=
INCS=	device_types.h
INCSDIR=${INCLUDEDIR}/mach/device

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

