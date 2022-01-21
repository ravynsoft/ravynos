# $FreeBSD$

NO_OBJ=
INCS=	hash_info.h \
	ipc_info.h \
	lockgroup_info.h \
	mach_debug_types.defs \
	mach_debug_types.h \
	page_info.h \
	vm_info.h \
	zone_info.h
INCSDIR=${INCLUDEDIR}/mach_debug

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor


distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

