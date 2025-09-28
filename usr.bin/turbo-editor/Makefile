.include <bsd.prog.mk>

#.MAKEFLAGS=
#GMAKE=                  ${DESTDIR}/legacy/bin/gmake

CXXFLAGS=       --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin -fPIC
CXX=            c++ ${CXXFLAGS}

all: build
build:
	CXX="${CXX}" CMAKE_ROOT=/usr cmake -S ${.CURDIR} -B ${MAKEOBJDIR} \
                -DCMAKE_INSTALL_PREFIX=${PREFIX}
	${MAKE} -C ${MAKEOBJDIR}

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
