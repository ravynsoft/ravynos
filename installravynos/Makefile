all: installravynos
clean:
build: installravynos

installravynos:
	${SUDO} mkdir -p ${TOPDIR}/dist
	${SUDO} tar cvJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${TOPDIR}/dist/ravynOS.txz .
	${SUDO} tar -C / -xvf ${TOPDIR}/dist/ravynOS.txz

.include <rvn.common.mk>
