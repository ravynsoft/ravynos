all: installravynos
clean:
build: installravynos

installravynos:
	${SUDO} mkdir -p ${TOPDIR}/dist
	${SUDO} tar cvJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${TOPDIR}/dist/ravynos.txz .
	${SUDO} tar -C / -xvf ${TOPDIR}/dist/ravynos.txz

.include <rvn.common.mk>
