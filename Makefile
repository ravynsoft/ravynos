# Primary makefile for the Helium OS

TOPDIR := ${.CURDIR}
OBJPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${TOPDIR}/freebsd-src/release
BSDCONFIG := GENERIC
BUILDROOT := ${OBJPREFIX}/buildroot
HELIUM_VERSION != head -1 ${TOPDIR}/version
HELIUM_CODENAME != tail -1 ${TOPDIR}/version
OSRELEASE := 12.2
FREEBSD_BRANCH := releng/${OSRELEASE}
MKINCDIR := -m/usr/share/mk -m${TOPDIR}/mk
CORES := 4

# Full release build with installation artifacts
world: prep freebsd helium release

prep:
	mkdir -p ${OBJPREFIX} ${TOPDIR}/dist ${BUILDROOT}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} \
	&& cd ../compile/${BSDCONFIG} && export MAKEOBJDIRPREFIX=${OBJPREFIX} \
	&& make depend)

${TOPDIR}/freebsd-src:
	cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout ${FREEBSD_BRANCH}

${OBJPREFIX}/.patched_bsd: patches/[0-9]*.patch
	(cd ${TOPDIR}/freebsd-src && git checkout -f ${FREEBSD_BRANCH}; \
	git branch -D helium/12 || true; \
	git checkout -b helium/12; \
	for patch in ${TOPDIR}/patches/[0-9]*.patch; do patch -p1 < $$patch; done; \
	git commit -a -m "patched")
	touch ${OBJPREFIX}/.patched_bsd

freebsd: kernel base

kernel: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make ${MFLAGS} -C ${TOPDIR}/freebsd-src buildkernel 

base: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make ${MFLAGS} -j${CORES} \
		-C ${TOPDIR}/freebsd-src buildworld

makepkg: packages-db-clean
	mkdir -p ${OBJPREFIX}/metadir
	sed -e 's/%%VERSION%%/${HELIUM_VERSION}/' <${TOPDIR}/+MANIFEST.helium \
		>${OBJPREFIX}/metadir/+MANIFEST
	cd ${BUILDROOT}; find -L . -not -type d |sed -e's/^.\///' >${OBJPREFIX}/pkg-plist
	INSTALL_AS_USER=1 PKG_DBDIR=${BUILDROOT}/var/db/pkg \
		pkg register -m ${OBJPREFIX}/metadir -f ${OBJPREFIX}/pkg-plist

packages-db-clean:
	rm -f ${BUILDROOT}/var/db/pkg/*

mv-pkgconfig:
	mkdir -p ${BUILDROOT}/usr/local/share
	tar -C ${BUILDROOT}/usr/lib -cpf pkgconfig | tar -C ${BUILDROOT}/usr/local/share -xpf -
	rm -rf ${BUILDROOT}/usr/lib/pkgconfig

helium: extradirs mkfiles libobjc2 libunwind frameworksclean frameworks copyfiles \
	mv-pkgconfig makepkg

# Update the build system with current source
install: installworld installkernel installhelium

installworld:
	sudo MAKEOBJDIRPREFIX=${OBJPREFIX} make -C ${TOPDIR}/freebsd-src installworld

installkernel:
	sudo MAKEOBJDIRPREFIX=${OBJPREFIX} make -C ${TOPDIR}/freebsd-src installkernel

installhelium: helium-package
	sudo tar -C / -xvf ${RLSDIR}/helium.txz

extradirs:
	rm -rf ${BUILDROOT}
	for x in System System/Library/Frameworks Library Users Applications Volumes; \
		do mkdir -p ${BUILDROOT}/$$x; \
	done
	ln -sf /usr/local/share/fonts ${BUILDROOT}/System/Library/Fonts
	mkdir -p ${BUILDROOT}/usr/bin
	ln -sf /usr/local/bin/zsh ${BUILDROOT}/usr/bin/zsh

mkfiles:
	mkdir -p ${BUILDROOT}/usr/share/mk
	cp -fv ${TOPDIR}/mk/*.mk ${BUILDROOT}/usr/share/mk/

copyfiles:
	cp -fvR ${TOPDIR}/etc ${BUILDROOT}
	sed -i_ -e "s/__VERSION__/${HELIUM_VERSION}/" -e "s/__CODENAME__/${HELIUM_CODENAME}/" ${BUILDROOT}/etc/motd
	rm -f ${BUILDROOT}/etc/motd_

libobjc2: .PHONY
	mkdir -p ${OBJPREFIX}/libobjc2
	cd ${OBJPREFIX}/libobjc2; cmake \
		-DCMAKE_C_FLAGS="-DBSD -D__HELIUM__ -DNO_SELECTOR_MISMATCH_WARNINGS" \
		-DCMAKE_BUILD_TYPE=Release \
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

_FRAMEWORK_TARGETS=
.if defined(FRAMEWORKS) && !empty(FRAMEWORKS)
.for fmwk in ${FRAMEWORKS}
_FRAMEWORK_TARGETS+=${fmwk}.framework
.endfor
.else
_FRAMEWORK_TARGETS=${.ALLTARGETS:M*.framework}
.endif
frameworks: 
	for fmwk in ${_FRAMEWORK_TARGETS}; do \
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

${TOPDIR}/ISO:
	git clone https://github.com/mszoek/ISO.git
	cd ISO && git checkout helium

${RLSDIR}/CocoaDemo.app.txz:
	make -C examples/app clean
	make -C examples/app 
	tar -C examples/app -cf ${.TARGET} CocoaDemo.app

desc_helium=Helium system
release: helium-package ${TOPDIR}/ISO ${RLSDIR}/CocoaDemo.app.txz
	rm -f ${RLSDIR}/packagesystem
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; sudo \
		make -C ${TOPDIR}/freebsd-src/release \
		desc_helium="${desc_helium}" NOSRC=true NOPORTS=true packagesystem 
	cd ISO && workdir=${OBJPREFIX} HELIUM=${TOPDIR} sudo ./build.sh hello Helium_${HELIUM_VERSION}
