# $FreeBSD$

.include <bsd.own.mk>

NO_OBJ=
INCS=	_pthread_attr_t.h \
	_pthread_cond_t.h \
	_pthread_condattr_t.h \
	_pthread_key_t.h \
	_pthread_mutex_t.h \
	_pthread_mutexattr_t.h \
	_pthread_once_t.h \
	_pthread_rwlock_t.h \
	_pthread_rwlockattr_t.h \
	_pthread_t.h \
	_pthread_types.h

INCSDIR=${INCLUDEDIR}/apple/sys/_pthread

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

