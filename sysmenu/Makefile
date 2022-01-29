WORLDTMP=	/usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/release/dist/airyx
EXTRA_CXXFLAGS=	"-F${WORLDTMP}/System/Library/Frameworks" \

all: .PHONY
	mkdir -p ${MAKEOBJDIR}
	echo '#define AIRYX_VERSION "${AIRYX_VERSION}"' > ${TOPDIR}/sysmenu/src/version.h
	echo '#define AIRYX_CODENAME "${AIRYX_CODENAME}"' >> ${TOPDIR}/sysmenu/src/version.h
	cmake -DCMAKE_CXX_FLAGS="${EXTRA_CXXFLAGS}" \
		-S ${TOPDIR}/sysmenu -B ${MAKEOBJDIR}
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} install

clean: .PHONY
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} clean


