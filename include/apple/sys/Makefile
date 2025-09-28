# $FreeBSD$

.include <bsd.own.mk>

NO_OBJ=
INCS=	_posix_availability.h \
	_symbol_aliasing.h \
	appleapiopts.h \
	fileport.h \
	kdebug.h \
	kern_control.h \
	kern_event.h \
	kern_memorystatus.h \
	proc_info.h \
	process_policy.h \
	resource.h \
	spawn.h \
	spawn_internal.h \
	sys_domain.h \

INCSDIR=${INCLUDEDIR}/apple/sys
SUBDIR=	_pthread _types

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

.include <bsd.prog.mk>
