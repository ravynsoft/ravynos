WORLDTMP=	/usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/release/dist/airyx
all: .PHONY
	mkdir -p ${MAKEOBJDIR}
	echo '#define AIRYX_VERSION "${AIRYX_VERSION}"' > ${TOPDIR}/sysmenu/src/version.h
	echo '#define AIRYX_CODENAME "${AIRYX_CODENAME}"' >> ${TOPDIR}/sysmenu/src/version.h
	cmake -DCMAKE_C_FLAGS="-F${WORLDTMP}/System/Library/Frameworks" \
	-S ${TOPDIR}/sysmenu -B ${MAKEOBJDIR}
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} install
