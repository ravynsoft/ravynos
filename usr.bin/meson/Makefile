.include <bsd.prog.mk>

PROG=                   meson

all: meson
meson:
	${.CURDIR}/packaging/create_zipapp.py --outfile meson.pyz \
            --interpreter '/usr/bin/env python3' ${.CURDIR}
	cp -fv meson.pyz ${DESTDIR}${PREFIX}/bin/meson
	chmod a+x ${DESTDIR}${PREFIX}/bin/meson

install:
includes:
