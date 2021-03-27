# Primary makefile for the Helium OS

TOPDIR := ${.CURDIR}
OBJPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${OBJPREFIX}${TOPDIR}/freebsd-src/release
BSDCONFIG := GENERIC
BUILDROOT := ${OBJPREFIX}/buildroot
HELIUM_VERSION != cat ${TOPDIR}/version
BRANCH_OVERRIDE := HELIUM_${HELIUM_VERSION}
FREEBSD_BRANCH := releng/12.2
MKINCDIR := -m/usr/share/mk -m${TOPDIR}/mk
CORES := 4

# Incremental build for quick tests or system update
build: prep freebsd-noclean helium

# Full release build with installation artifacts
world: prep freebsd helium release


prep:
	mkdir -p ${OBJPREFIX} ${RLSDIR} ${TOPDIR}/dist ${BUILDROOT}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} \
	&& cd ../compile/${BSDCONFIG} && export MAKEOBJDIRPREFIX=${OBJPREFIX} \
	&& make depend)

checkout:
	test -d ${TOPDIR}/freebsd-src || \
		(cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout ${FREEBSD_BRANCH})

patchbsd: patches/*.patch
	(cd ${TOPDIR}/freebsd-src && git checkout -f ${FREEBSD_BRANCH}; \
	git branch -D helium/12 || true; \
	git checkout -b helium/12; \
	for patch in ${TOPDIR}/patches/*.patch; do patch -p1 < $$patch; done; \
	git commit -a -m "patched")

freebsd: checkout patchbsd ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make BRANCH_OVERRIDE=${BRANCH_OVERRIDE} \
		-C ${TOPDIR}/freebsd-src buildkernel 
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make \
		-j${CORES} -C ${TOPDIR}/freebsd-src buildworld

freebsd-noclean:
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make BRANCH_OVERRIDE=${BRANCH_OVERRIDE} \
		-C ${TOPDIR}/freebsd-src -DNO_CLEAN buildkernel
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make \
		-j${CORES} -C ${TOPDIR}/freebsd-src -DNO_CLEAN buildworld

kernel-noclean:
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make BRANCH_OVERRIDE=${BRANCH_OVERRIDE} \
		-C ${TOPDIR}/freebsd-src -DNO_CLEAN buildkernel

usr-noclean:
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make -j${CORES} \
		-C ${TOPDIR}/freebsd-src -DNO_CLEAN buildworld

XORG=https://www.x.org/releases/individual/xserver/xorg-server-1.20.10.tar.bz2 \
     https://www.x.org/releases/individual/xcb/libxcb-1.14.tar.gz \
	 https://www.x.org/releases/individual/xcb/xcb-proto-1.14.tar.gz \
	 
packages:
	mkdir -p ${OBJPREFIX}/pkg
	pkg fetch -U -o ${OBJPREFIX}/pkg -d -y openjpeg tiff png freetype2 fontconfig cairo xorg

helium: extradirs mkfiles libobjc2 libunwind frameworksclean frameworks copyfiles

# Update the build system with current source
install: installworld installkernel installhelium

installworld:
	sudo -E MAKEOBJDIRPREFIX=${OBJPREFIX} make -C ${TOPDIR}/freebsd-src installworld

installkernel:
	sudo -E MAKEOBJDIRPREFIX=${OBJPREFIX} make -C ${TOPDIR}/freebsd-src installkernel

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

copyfiles:
	cp -fvR ${TOPDIR}/etc ${BUILDROOT}

libobjc2: .PHONY
	mkdir -p ${OBJPREFIX}/libobjc2
	cd ${OBJPREFIX}/libobjc2; cmake \
		-DCMAKE_C_FLAGS="-DBSD -D__HELIUM__" \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=false -DLEGACY_COMPAT=false \
		${TOPDIR}/libobjc2
	make -C ${OBJPREFIX}/libobjc2 DESTDIR=${BUILDROOT} install

libunwind: .PHONY
	cd libunwind-1.5.0 && ./configure --prefix=/usr --enable-coredump --enable-ptrace --enable-cxx-exceptions \
		--enable-block-signals --enable-debug-frame && make -j${CORES}
	make -C libunwind-1.5.0 install prefix=${BUILDROOT}/usr

frameworksclean:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/*.framework
	for fmwk in ${.ALLTARGETS:M*.framework:R}; do \
		make ${MKINCDIR} -C $$fmwk clean; \
		rm -rf $$fmwk/$$fmwk.framework; \
	done
	rm -rf Foundation/Headers

frameworks: 
	for fmwk in ${.ALLTARGETS:M*.framework}; do \
		make ${MKINCDIR} $$fmwk; done

marshallheaders:
	make -C Foundation marshallheaders

# DO NOT change the order of these 4 frameworks!
CoreFoundation.framework: marshallheaders
	rm -rf CoreFoundation/${.TARGET}
	make -C CoreFoundation BUILDROOT=${BUILDROOT} clean
	make -C CoreFoundation BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CFNetwork.framework:
	rm -rf CFNetwork/${.TARGET}
	make -C CFNetwork BUILDROOT=${BUILDROOT} clean
	make -C CFNetwork BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Foundation.framework:
	rm -rf Foundation/${.TARGET}
	make -C Foundation BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/NSException/NSRaise.h ${TOPDIR}/AppKit

ApplicationServices.framework:
	rm -rf ApplicationServices/${.TARGET}
	make -C ApplicationServices BUILDROOT=${BUILDROOT} clean
	make -C ApplicationServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreServices.framework:
	rm -rf CoreServices/${.TARGET}
	make -C CoreServices BUILDROOT=${BUILDROOT} clean
	make -C CoreServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreData.framework:
	rm -rf CoreData/${.TARGET}
	make -C CoreData BUILDROOT=${BUILDROOT} clean
	make -C CoreData BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Onyx2D.framework:
	rm -rf Onyx2D/${.TARGET}
	make -C Onyx2D BUILDROOT=${BUILDROOT} clean
	make -C Onyx2D BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

OpenGL.framework:
	rm -rf OpenGL/${.TARGET}
	make -C OpenGL BUILDROOT=${BUILDROOT} clean
	make -C OpenGL BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreGraphics.framework:
	rm -rf CoreGraphics/${.TARGET}
	make -C CoreGraphics BUILDROOT=${BUILDROOT} clean
	make -C CoreGraphics BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/CGEvent.h ${TOPDIR}/AppKit

CoreText.framework:
	rm -rf CoreText/${.TARGET}
	make -C CoreText BUILDROOT=${BUILDROOT} clean
	make -C CoreText BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/KTFont.h ${TOPDIR}/AppKit

QuartzCore.framework:
	rm -rf QuartzCore/${.TARGET}
	make -C QuartzCore BUILDROOT=${BUILDROOT} clean
	make -C QuartzCore BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Cocoa.framework:
	rm -rf Cocoa/${.TARGET}
	make -C Cocoa BUILDROOT=${BUILDROOT} clean
	make -C Cocoa BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

AppKit.framework:
	rm -rf AppKit/${.TARGET}
	make -C AppKit BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks


helium-package:
	tar cJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/helium.txz .

desc_helium=Helium system
release: helium-package
	rm -f ${RLSDIR}/disc1.iso ${RLSDIR}/memstick.img 
	if [ -d ${RLSDIR}/disc1 ]; then \
		sudo chflags -R noschg,nouchg ${RLSDIR}/disc1 && sudo rm -rf ${RLSDIR}/disc1; \
	fi
	rm -f ${RLSDIR}/packagesystem
	sudo -E MAKEOBJDIRPREFIX=${OBJPREFIX} \
		make -C ${TOPDIR}/freebsd-src/release \
		desc_helium="${desc_helium}" NOSRC=true NOPORTS=true \
		packagesystem disc1.iso memstick
	cp -fv ${RLSDIR}/disc1.iso ${RLSDIR}/memstick.img ${TOPDIR}/dist/
