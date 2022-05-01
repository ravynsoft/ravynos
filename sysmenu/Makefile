WORLDTMP=	/usr/obj/${TOPDIR}/${MACHINE}.${MACHINE}/release/dist/ravynos
EXTRA_CXXFLAGS=	"-F${WORLDTMP}/System/Library/Frameworks" \

all: .PHONY
	mkdir -p ${MAKEOBJDIR}
	echo '#define RAVYNOS_VERSION "${RAVYNOS_VERSION}"' > ${TOPDIR}/sysmenu/src/version.h
	echo '#define RAVYNOS_CODENAME "${RAVYNOS_CODENAME}"' >> ${TOPDIR}/sysmenu/src/version.h
	cmake -DCMAKE_CXX_FLAGS="${EXTRA_CXXFLAGS}" \
		-S ${TOPDIR}/sysmenu -B ${MAKEOBJDIR}
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} install

clean: .PHONY
	make -C ${MAKEOBJDIR} DESTDIR=${BUILDROOT} clean


