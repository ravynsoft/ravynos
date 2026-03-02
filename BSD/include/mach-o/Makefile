# $FreeBSD$

NO_OBJ=
INCS=	arch.h \
	dyld_images.h \
	dyld_priv.h \
	dyld_process_info.h \
	dyld-interposing.h \
	dyld.h \
	fat.h \
	fixup-chains.h \
	getsect.h \
	gmon.h \
	kld.h \
	ldsyms.h \
	loader.h \
	nlist.h \
	ranlib.h \
	redo_prebinding.h \
	reloc.h \
	rld_state.h \
	rld.h \
	sarld.h \
	stab.h \
	swap.h
INCSDIR=${INCLUDEDIR}/mach-o
SUBDIR=	arm arm64 hppa i386 i860 m68k m88k ppc sparc x86_64

includes:
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 ${DESTDIR}${INCSDIR}
.for h in ${INCS}
	${INSTALL} -C -o ${BINOWN} -g ${BINGRP} -m 0444 ${.CURDIR}/$h ${DESTDIR}${INCSDIR}
.endfor

distprep:
	mkdir -pv ${DISTDIR}/base/${INCSDIR}

.include <bsd.prog.mk>

distribute: distprep

