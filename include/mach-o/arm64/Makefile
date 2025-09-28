# $FreeBSD$

NO_OBJ=
INCS=	reloc.h
INCSDIR=${INCLUDEDIR}/mach-o/arm64
SUBDIR=

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

installdirs:
	mkdir -p ${DESTDIR}${INCSDIR}

distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep
install: installdirs
