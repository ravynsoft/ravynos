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
CORES != sysctl -n hw.ncpu
SUDO != test "$$USER" == "root" && echo "" || echo "sudo"

# Full release build with installation artifacts
world: prep freebsd airyx release

prep: cleanroot
	mkdir -p ${OBJPREFIX} ${TOPDIR}/dist ${BUILDROOT}
	mkdir -p ${BUILDROOT}/etc ${BUILDROOT}/var/run ${BUILDROOT}/usr/sbin
	${SUDO} cp -f ${TOPDIR}/make.conf ${TOPDIR}/resolv.conf ${BUILDROOT}/etc/
	${SUDO} cp -f /var/run/ld-elf.so.hints ${BUILDROOT}/var/run
	${SUDO} cp -f /usr/local/sbin/pkg-static ${BUILDROOT}/usr/sbin
	for x in System System/Library/Frameworks Library Users Applications Volumes; \
		do mkdir -p ${BUILDROOT}/$$x; \
	done

cleanroot:
	if [ -d ${BUILDROOT} ]; then \
		${SUDO} chflags -R noschg,nouchg ${BUILDROOT}; \
		${SUDO} rm -rf ${BUILDROOT}; \
	fi

getports:
	${SUDO} portsnap auto
	${SUDO} ${TOPDIR}/Tools/patch-ports.sh
	${SUDO} cp -f ${TOPDIR}/patches/patch-conf.d_link__confs.py /usr/ports/x11-fonts/fontconfig/files/
	${SUDO} mkdir /usr/ports/graphics/jpeg-turbo/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-cmakescripts_GNUInstallDirs.cmake /usr/ports/graphics/jpeg-turbo/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-meson.build /usr/ports/sysutils/polkit/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-freebsd_Makefile /usr/ports/shells/bash-completion/files/
	${SUDO} mkdir -p /usr/ports/sysutils/bsdisks/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-CMakeLists.txt /usr/ports/sysutils/bsdisks/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-mysql57_install__layout.cmake /usr/ports/databases/mysql57-client/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-webcamd-Makefile /usr/ports/multimedia/webcamd/files/
	${SUDO} mkdir -p /usr/ports/audio/lilv/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-waflib_extras_autowaf.py /usr/ports/audio/lilv/files/
	${SUDO} mkdir /usr/ports/distfiles

# Prepare the chroot jail for our ports builds
prepports:
	if [ -d ${PORTSROOT} ]; then \
		${SUDO} chflags -R noschg,nouchg ${PORTSROOT}; \
		${SUDO} rm -rf ${PORTSROOT}; \
	fi
	mkdir -p ${PORTSROOT}/etc ${PORTSROOT}/var/run ${PORTSROOT}/usr/sbin
	${SUDO} cp -f ${TOPDIR}/make.conf ${TOPDIR}/resolv.conf ${PORTSROOT}/etc/
	${SUDO} cp -f /var/run/ld-elf.so.hints ${PORTSROOT}/var/run
	${SUDO} cp -f /usr/local/sbin/pkg-static ${PORTSROOT}/usr/sbin
	${SUDO} tar xvf ${RLSDIR}/base.txz -C ${PORTSROOT}
	${SUDO} ln -s libncurses.so ${PORTSROOT}/usr/lib/libncurses.so.6

/usr/ports/{archivers,audio,devel,dns,emulators,graphics,misc,multimedia,net,security,shells,sysutils,textproc,x11,x11-fonts,x11-fm,x11-themes}/*: .PHONY
	${SUDO} ${MAKE} -C ${.TARGET} DESTDIR=${PORTSROOT} install

mountsrc:
	${SUDO} mount_nullfs ${TOPDIR}/freebsd-src/ ${PORTSROOT}/usr/src

umountsrc:
	${SUDO} umount ${PORTSROOT}/usr/src

zsh: /usr/ports/shells/zsh
	${SUDO} ln -f ${PORTSROOT}/usr/bin/zsh ${PORTSROOT}/bin/zsh

plasma: /usr/ports/x11/plasma5-plasma /usr/ports/x11/konsole /usr/ports/x11/sddm /usr/ports/x11-fm/dolphin
xorg: /usr/ports/x11/xorg /usr/ports/x11-themes/adwaita-icon-theme /usr/ports/devel/desktop-file-utils
misc: /usr/ports/archivers/brotli /usr/ports/graphics/argyllcms /usr/ports/multimedia/gstreamer1-plugins-all
misc2: /usr/ports/x11/zenity /usr/ports/sysutils/cpdup /usr/ports/audio/freedesktop-sound-theme /usr/ports/sysutils/fusefs-libs mountsrc /usr/ports/graphics/gpu-firmware-kmod /usr/ports/sysutils/iichid /usr/ports/net/libdnet /usr/ports/archivers/libmspack /usr/ports/security/libretls /usr/ports/devel/libsigc++20 /usr/ports/multimedia/libva-intel-driver /usr/ports/dns/nss_mdns /usr/ports/emulators/open-vm-tools /usr/ports/net/openntpd /usr/ports/sysutils/pv /usr/ports/misc/usbids /usr/ports/misc/utouch-kmod umountsrc /usr/ports/net/wpa_supplicant_gui /usr/ports/devel/xdg-user-dirs
buildports: zsh xorg plasma misc misc2

makepackages:
	${SUDO} rm -rf /usr/ports/packages
	${SUDO} mkdir -p /usr/ports/packages
	${SUDO} mount_nullfs /usr/ports/packages ${PORTSROOT}/mnt
	${SUDO} chroot ${PORTSROOT} /bin/sh -c '/usr/sbin/pkg-static create -a -o /mnt'
	${SUDO} umount ${PORTSROOT}/mnt
	${SUDO} pkg repo -o /usr/ports/packages /usr/ports/packages

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

airyx: mkfiles libobjc2 libunwind frameworksclean frameworks copyfiles
	tar -C ${BUILDROOT}/usr/lib -cpf pkgconfig | tar -C ${BUILDROOT}/usr/share -xpf -
	rm -rf ${BUILDROOT}/usr/lib/pkgconfig

# Update the build system with current source
install: installworld installkernel installairyx

installworld:
	${SUDO} -E MAKEOBJDIRPREFIX=${OBJPREFIX} ${MAKE} -C ${TOPDIR}/freebsd-src installworld

installkernel:
	${SUDO} -E MAKEOBJDIRPREFIX=${OBJPREFIX} ${MAKE} -C ${TOPDIR}/freebsd-src installkernel

installairyx: airyx-package
	${SUDO} tar -C / -xvf ${RLSDIR}/airyx.txz

copyfiles:
	cp -fvR ${TOPDIR}/etc ${BUILDROOT}
	sed -i_ -e "s/__VERSION__/${AIRYX_VERSION}/" -e "s/__CODENAME__/${AIRYX_CODENAME}/" ${BUILDROOT}/etc/motd
	rm -f ${BUILDROOT}/etc/motd_

mkfiles:
	mkdir -p ${BUILDROOT}/usr/share/mk
	cp -fv ${TOPDIR}/mk/*.mk ${BUILDROOT}/usr/share/mk/

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
	${SUDO} tar cvJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/airyx.txz .

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
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; ${SUDO} -E \
		${MAKE} -C ${TOPDIR}/freebsd-src/release NOSRC=true NOPORTS=true packagesystem 

iso:
	cp -f ${TOPDIR}/version.txt ${TOPDIR}/ISO/overlays/ramdisk/version
	cd ${TOPDIR}/ISO && workdir=${OBJPREFIX} AIRYX=${TOPDIR} ${SUDO} -E ./build.sh kde Airyx_${AIRYX_VERSION}

release: airyx-package ${TOPDIR}/ISO ${RLSDIR}/CocoaDemo.app.txz packagesystem iso
