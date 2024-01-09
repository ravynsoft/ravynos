.include <bsd.prog.mk>

all:
	cmake -G "Unix Makefiles" -S ${.CURDIR} -B ${MAKEOBJDIR}
	make ${.MAKEFLAGS}
	make install DESTDIR=${DESTDIR}

install:
	make install DESTDIR=${DESTDIR}
