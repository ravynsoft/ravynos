all: .PHONY
	mkdir -p ${MAKEOBJDIR}
	echo '#define AIRYX_VERSION "${AIRYX_VERSION}"' > ${TOPDIR}/sysmenu/src/version.h
	echo '#define AIRYX_CODENAME "${AIRYX_CODENAME}"' >> ${TOPDIR}/sysmenu/src/version.h
	cmake -S ${TOPDIR}/sysmenu -B ${MAKEOBJDIR}
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} install
