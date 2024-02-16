.include <bsd.lib.mk>

.if ${MACHINE} == "amd64"
CMAKE_ARGS+=    -DCMAKE_SYSTEM_PROCESSOR=x86_64
.endif

all: build
build:
	cmake -S ${.CURDIR} -B ${MAKEOBJDIR} -DCMAKE_INSTALL_PREFIX=${PREFIX} \
                ${CMAKE_ARGS}
	${MAKE} -C ${MAKEOBJDIR}
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
