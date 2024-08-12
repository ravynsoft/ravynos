.include <bsd.prog.mk>

DESTDIR?=	${OBJTOP}/tmp
PREFIX?=	/usr

all:
build:
install:
	cd ${.CURDIR}; PYTHONPATH=${DESTDIR}${PREFIX}/lib/python3.10/ \
		LD_LIBRARY_PATH=${OBJTOP}/usr.bin/python \
	        CFLAGS=-I${DESTDIR}${PREFIX}/include/python3.10 \
                ${DESTDIR}${PREFIX}/bin/python3 setup.py install 
	rm -rf ${.CURDIR}/Mako.egg-info ${.CURDIR}/build

