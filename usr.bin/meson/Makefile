.include <bsd.prog.mk>

PROG=                   meson

all: meson
meson:
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib \
	    ${.CURDIR}/packaging/create_zipapp.py --outfile meson.pyz \
            --interpreter '/usr/bin/env python3' ${.CURDIR}
	cp -fv meson.pyz ${DESTDIR}${PREFIX}/bin/meson
	chmod a+x ${DESTDIR}${PREFIX}/bin/meson

install:
includes:
