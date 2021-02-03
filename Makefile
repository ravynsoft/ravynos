# Primary makefile for the Helium OS

TOPDIR := ${.CURDIR}
MAKEOBJDIRPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${MAKEOBJDIRPREFIX}${TOPDIR}/freebsd-src/${MACHINE}.${MACHINE}/release
BSDCONFIG := GENERIC
BUILDROOT := ${MAKEOBJDIRPREFIX}/buildroot

# Incremental build for quick tests or system update
build: prep freebsd-noclean helium

# Full release build with installation artifacts
world: prep freebsd helium release


prep:
	mkdir -p ${MAKEOBJDIRPREFIX} ${RLSDIR} ${TOPDIR}/dist ${BUILDROOT}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} 
		\ && cd ../compile/${BSDCONFIG} && make depend)

checkout:
	test -d ${TOPDIR}/freebsd-src || \
		(cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout stable/12)

freebsd: checkout ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; make -C ${TOPDIR}/freebsd-src buildkernel buildworld

freebsd-noclean:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; make -C ${TOPDIR}/freebsd-src -DNO_CLEAN buildkernel buildworld

helium: extradirs mkfiles libobjc2

# Update the build system with current source
install: installworld installkernel

installworld:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; sudo make -C ${TOPDIR}/freebsd-src installworld

installkernel:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; sudo make -C ${TOPDIR}/freebsd-src installkernel

extradirs:
	rm -rf ${BUILDROOT}
	for x in System System/Library Library Users Applications Volumes; \
		do mkdir -p ${BUILDROOT}/$$x; \
	done

mkfiles:
	mkdir -p ${MAKEOBJDIRPREFIX}/buildroot/usr/share/mk
	cp -fv ${TOPDIR}/mk/*.mk ${MAKEOBJDIRPREFIX}/buildroot/usr/share/mk/

libobjc2: .PHONY
	mkdir -p ${MAKEOBJDIRPREFIX}/libobjc2
	cd ${MAKEOBJDIRPREFIX}/libobjc2; cmake -DCMAKE_INSTALL_PREFIX=/usr ${TOPDIR}/libobjc2
	make -C ${MAKEOBJDIRPREFIX}/libobjc2 DESTDIR=${BUILDROOT} install

helium-package:
	tar cJ -C ${MAKEOBJDIRPREFIX}/buildroot --gid 0 --uid 0 -f ${RLSDIR}/helium.txz .

desc_helium=Helium system
release: helium-package
	rm -f ${RLSDIR}/disc1.iso
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX} desc_helium="${desc_helium}"; \
		sudo -E make -C ${TOPDIR}/freebsd-src/release disc1.iso memstick
	cp -fv ${RLSDIR}/*.img ${RLSDIR}/*.iso ${TOPDIR}/dist/
