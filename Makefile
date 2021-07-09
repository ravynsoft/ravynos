# Primary makefile for the Airyx OS

TOPDIR := ${.CURDIR}
OBJPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${TOPDIR}/freebsd-src/release
BSDCONFIG := GENERIC
BUILDROOT := ${OBJPREFIX}/buildroot
PORTSROOT := ${OBJPREFIX}/portsroot
AIRYX_VERSION != head -1 ${TOPDIR}/version.txt
AIRYX_CODENAME != tail -1 ${TOPDIR}/version.txt
OSRELEASE := 12.2
FREEBSD_BRANCH := stable/${OSRELEASE:R}
MKINCDIR := -m/usr/share/mk -m${TOPDIR}/mk
CORES := 8

# Full release build with installation artifacts
world: prep freebsd airyx release

prep:
	mkdir -p ${OBJPREFIX} ${TOPDIR}/dist ${BUILDROOT}
	mkdir -p ${BUILDROOT}/etc ${BUILDROOT}/var/run ${BUILDROOT}/usr/sbin
	sudo cp -f ${TOPDIR}/make.conf ${TOPDIR}/resolv.conf ${BUILDROOT}/etc/
	sudo cp -f /var/run/ld-elf.so.hints ${BUILDROOT}/var/run
	sudo cp -f /usr/local/sbin/pkg-static ${BUILDROOT}/usr/sbin

copybase:
	sudo tar xvf ${RLSDIR}/base.txz -C ${BUILDROOT}

zsh:
	sudo ${MAKE} -C /usr/ports/shells/zsh DESTDIR=${BUILDROOT} clean rmconfig-recursive install

cleanroot:
	sudo chflags -R noschg,nouchg ${BUILDROOT}
	sudo rm -rf ${BUILDROOT}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} \
	&& cd ../compile/${BSDCONFIG} && export MAKEOBJDIRPREFIX=${OBJPREFIX} \
	&& ${MAKE} depend)

${TOPDIR}/freebsd-src:
	cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout ${FREEBSD_BRANCH}

${OBJPREFIX}/.patched_bsd: patches/[0-9]*.patch
	(cd ${TOPDIR}/freebsd-src && git checkout -f ${FREEBSD_BRANCH}; \
	git branch -D airyx/12 || true; \
	git checkout -b airyx/12; \
	for patch in ${TOPDIR}/patches/[0-9]*.patch; do patch -p1 < $$patch; done; \
	git commit -a -m "patched")
	touch ${OBJPREFIX}/.patched_bsd

freebsd: kernel base

kernel: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; ${MAKE} ${MFLAGS} -C ${TOPDIR}/freebsd-src buildkernel 

base: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; ${MAKE} ${MFLAGS} -j${CORES} \
		-C ${TOPDIR}/freebsd-src buildworld

makepkg: packages-db-clean
	mkdir -p ${OBJPREFIX}/metadir
	sed -e 's/%%VERSION%%/${AIRYX_VERSION}/' <${TOPDIR}/+MANIFEST.airyx \
		>${OBJPREFIX}/metadir/+MANIFEST
	cd ${BUILDROOT}; find -L . -not -type d |sed -e's/^.\///' >${OBJPREFIX}/pkg-plist
	INSTALL_AS_USER=1 PKG_DBDIR=${BUILDROOT}/var/db/pkg \
		pkg register -m ${OBJPREFIX}/metadir -f ${OBJPREFIX}/pkg-plist

packages-db-clean:
	rm -f ${BUILDROOT}/var/db/pkg/*

mv-pkgconfig:
	mkdir -p ${BUILDROOT}/usr/share
	tar -C ${BUILDROOT}/usr/lib -cpf pkgconfig | tar -C ${BUILDROOT}/usr/share -xpf -
	rm -rf ${BUILDROOT}/usr/lib/pkgconfig

airyx: extradirs mkfiles libobjc2 libunwind frameworksclean frameworks copyfiles \
	mv-pkgconfig 

# Update the build system with current source
install: installworld installkernel installairyx

installworld:
	sudo -E MAKEOBJDIRPREFIX=${OBJPREFIX} ${MAKE} -C ${TOPDIR}/freebsd-src installworld

installkernel:
	sudo -E MAKEOBJDIRPREFIX=${OBJPREFIX} ${MAKE} -C ${TOPDIR}/freebsd-src installkernel

installairyx: airyx-package
	sudo tar -C / -xvf ${RLSDIR}/airyx.txz

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
	sed -i_ -e "s/__VERSION__/${AIRYX_VERSION}/" -e "s/__CODENAME__/${AIRYX_CODENAME}/" ${BUILDROOT}/etc/motd
	rm -f ${BUILDROOT}/etc/motd_

libobjc2: .PHONY
	mkdir -p ${OBJPREFIX}/libobjc2
	cd ${OBJPREFIX}/libobjc2; cmake \
		-DCMAKE_C_FLAGS=" -D__AIRYX__ -DNO_SELECTOR_MISMATCH_WARNINGS" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DOLDABI_COMPAT=false -DLEGACY_COMPAT=false \
		${TOPDIR}/libobjc2
	${MAKE} -C ${OBJPREFIX}/libobjc2 DESTDIR=${BUILDROOT} install

libunwind: .PHONY
	cd ${TOPDIR}/libunwind-1.5.0 && ./configure --prefix=/usr --enable-coredump --enable-ptrace --enable-cxx-exceptions \
		--enable-block-signals --enable-debug-frame && ${MAKE} -j${CORES}
	${MAKE} -C ${TOPDIR}/libunwind-1.5.0 install prefix=${BUILDROOT}/usr

frameworksclean:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/*.framework
	for fmwk in ${.ALLTARGETS:M*.framework:R}; do \
		${MAKE} ${MKINCDIR} -C ${TOPDIR}/$$fmwk clean; \
		rm -rf ${TOPDIR}/$$fmwk/$$fmwk.framework; \
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
		${MAKE} ${MKINCDIR} -C ${TOPDIR} $$fmwk; done

marshallheaders:
	${MAKE} -C ${TOPDIR}/Foundation marshallheaders

# DO NOT change the order of these 4 frameworks!
CoreFoundation.framework: marshallheaders
	rm -rf ${TOPDIR}/CoreFoundation/${.TARGET}
	${MAKE} -C ${TOPDIR}/CoreFoundation BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CoreFoundation BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CFNetwork.framework:
	rm -rf ${TOPDIR}/CFNetwork/${.TARGET}
	${MAKE} -C ${TOPDIR}/CFNetwork BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CFNetwork BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Foundation.framework:
	rm -rf ${TOPDIR}/Foundation/${.TARGET}
	${MAKE} -C ${TOPDIR}/Foundation BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/NSException/NSRaise.h ${TOPDIR}/AppKit

ApplicationServices.framework:
	rm -rf ${TOPDIR}/ApplicationServices/${.TARGET}
	${MAKE} -C ${TOPDIR}/ApplicationServices BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/ApplicationServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreServices.framework:
	rm -rf ${TOPDIR}/CoreServices/${.TARGET}
	${MAKE} -C ${TOPDIR}/CoreServices BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CoreServices BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreData.framework:
	rm -rf ${TOPDIR}/CoreData/${.TARGET}
	${MAKE} -C ${TOPDIR}/CoreData BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CoreData BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Onyx2D.framework:
	rm -rf ${TOPDIR}/Onyx2D/${.TARGET}
	${MAKE} -C ${TOPDIR}/Onyx2D BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/Onyx2D BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

OpenGL.framework:
	rm -rf ${TOPDIR}/OpenGL/${.TARGET}
	${MAKE} -C ${TOPDIR}/OpenGL BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/OpenGL BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

CoreGraphics.framework:
	rm -rf ${TOPDIR}/CoreGraphics/${.TARGET}
	${MAKE} -C ${TOPDIR}/CoreGraphics BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CoreGraphics BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/CGEvent.h ${TOPDIR}/AppKit

CoreText.framework:
	rm -rf ${TOPDIR}/CoreText/${.TARGET}
	${MAKE} -C ${TOPDIR}/CoreText BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/CoreText BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks
	cp -vf ${TOPDIR}/${.TARGET:R}/KTFont.h ${TOPDIR}/AppKit

QuartzCore.framework:
	rm -rf ${TOPDIR}/QuartzCore/${.TARGET}
	${MAKE} -C ${TOPDIR}/QuartzCore BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/QuartzCore BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

Cocoa.framework:
	rm -rf ${TOPDIR}/Cocoa/${.TARGET}
	${MAKE} -C ${TOPDIR}/Cocoa BUILDROOT=${BUILDROOT} clean
	${MAKE} -C ${TOPDIR}/Cocoa BUILDROOT=${BUILDROOT}
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

AppKit.framework:
	rm -rf ${TOPDIR}/AppKit/${.TARGET}
	${MAKE} -C ${TOPDIR}/AppKit BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

DBusKit.framework:
	rm -rf ${TOPDIR}/DBusKit/${.TARGET}
	${MAKE} -C ${TOPDIR}/DBusKit BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

LaunchServices.framework:
	rm -rf ${TOPDIR}/LaunchServices/${.TARGET}
	${MAKE} -C ${TOPDIR}/LaunchServices BUILDROOT=${BUILDROOT} clean build
	cp -Rvf ${TOPDIR}/${.TARGET:R}/${.TARGET} ${BUILDROOT}/System/Library/Frameworks

airyx-package:
	tar cJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/airyx.txz .

${TOPDIR}/ISO:
	cd ${TOPDIR} && git clone https://github.com/mszoek/ISO.git
	cd ${TOPDIR}/ISO && git checkout airyx

${RLSDIR}/CocoaDemo.app.txz:
	${MAKE} -C ${TOPDIR}/examples/app clean
	${MAKE} -C ${TOPDIR}/examples/app 
	tar -C ${TOPDIR}/examples/app -cf ${.TARGET} CocoaDemo.app

desc_airyx=Airyx system
packagesystem:
	rm -f ${RLSDIR}/packagesystem
	cp -f ${TOPDIR}/version ${TOPDIR}/ISO/overlays/ramdisk
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; sudo -E \
		${MAKE} -C ${TOPDIR}/freebsd-src/release NOSRC=true NOPORTS=true packagesystem 

iso:
	cp -f ${TOPDIR}/version.txt ${TOPDIR}/ISO/overlays/ramdisk/version
	cd ${TOPDIR}/ISO && workdir=${OBJPREFIX} AIRYX=${TOPDIR} sudo -E ./build.sh kde Airyx_${AIRYX_VERSION}

release: airyx-package ${TOPDIR}/ISO ${RLSDIR}/CocoaDemo.app.txz packagesystem iso
