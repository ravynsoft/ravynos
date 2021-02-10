# Primary makefile for the Helium OS

TOPDIR := ${.CURDIR}
MAKEOBJDIRPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${MAKEOBJDIRPREFIX}${TOPDIR}/freebsd-src/${MACHINE}.${MACHINE}/release
BSDCONFIG := GENERIC
BUILDROOT := ${MAKEOBJDIRPREFIX}/buildroot

.MAKEFLAGS+= -m/usr/share/mk -m${TOPDIR}/mk

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

helium: extradirs mkfiles libobjc2 frameworksclean frameworks

# Update the build system with current source
install: installworld installkernel installhelium

installworld:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; sudo make -C ${TOPDIR}/freebsd-src installworld

installkernel:
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX}; sudo make -C ${TOPDIR}/freebsd-src installkernel

installhelium: helium-package
	sudo tar -C / -xvf ${RLSDIR}/helium.txz

extradirs:
	rm -rf ${BUILDROOT}
	for x in System System/Library/Frameworks Library Users Applications Volumes; \
		do mkdir -p ${BUILDROOT}/$$x; \
	done

mkfiles:
	mkdir -p ${BUILDROOT}/usr/share/mk
	cp -fv ${TOPDIR}/mk/*.mk ${BUILDROOT}/usr/share/mk/

libobjc2: .PHONY
	mkdir -p ${MAKEOBJDIRPREFIX}/libobjc2
	cd ${MAKEOBJDIRPREFIX}/libobjc2; cmake \
		-DCMAKE_C_FLAGS="-DBSD -DFREEBSD" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=true -DLEGACY_COMPAT=true \
		${TOPDIR}/libobjc2
	make -C ${MAKEOBJDIRPREFIX}/libobjc2 DESTDIR=${BUILDROOT} install


frameworksclean:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/*.framework
	for fmwk in ${.ALLTARGETS:M*.framework:R}; do \
		make -C $$fmwk clean; \
	done

frameworks: 
	for fmwk in ${.ALLTARGETS:M*.framework}; do make $$fmwk; done

Foundation.framework:
	rm -rf Foundation/${.TARGET}
	make -C Foundation BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreFoundation.framework:
	rm -rf CoreFoundation/${.TARGET}
	make -C CoreFoundation BUILDROOT=${BUILDROOT} clean
	make -C CoreFoundation BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreServices.framework:
	rm -rf CoreServices/${.TARGET}
	make -C CoreServices BUILDROOT=${BUILDROOT} clean
	make -C CoreServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CFNetwork.framework:
	rm -rf CFNetwork/${.TARGET}
	make -C CFNetwork BUILDROOT=${BUILDROOT} clean
	make -C CFNetwork BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

helium-package:
	tar cJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/helium.txz .

desc_helium=Helium system
release: helium-package
	rm -f ${RLSDIR}/disc1.iso
	export MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX} desc_helium="${desc_helium}"; \
		sudo -E make -C ${TOPDIR}/freebsd-src/release disc1.iso memstick
	cp -fv ${RLSDIR}/*.img ${RLSDIR}/*.iso ${TOPDIR}/dist/
