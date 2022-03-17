all: installairyx
clean:
build: installairyx

installairyx:
	${SUDO} mkdir -p ${TOPDIR}/dist
	${SUDO} tar cvJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${TOPDIR}/dist/airyx.txz .
	${SUDO} tar -C / -xvf ${TOPDIR}/dist/airyx.txz

.include <airyx.common.mk>
