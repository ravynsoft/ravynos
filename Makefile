# Primary makefile for the Helium OS

TOPDIR := ${.CURDIR}
OBJPREFIX := ${HOME}/obj.${MACHINE}
RLSDIR := ${TOPDIR}/freebsd-src/release
BSDCONFIG := GENERIC
BUILDROOT := ${OBJPREFIX}/buildroot
HELIUM_VERSION != cat ${TOPDIR}/version
BRANCH_OVERRIDE := HELIUM_${HELIUM_VERSION}
OSRELEASE := 12.2
FREEBSD_BRANCH := releng/${OSRELEASE}
MKINCDIR := -m/usr/share/mk -m${TOPDIR}/mk
PORTSDIR := ${TOPDIR}/ports
CORES := 4

# Incremental build for quick tests or system update
build: prep freebsd-noclean helium

# Full release build with installation artifacts
world: prep freebsd fetchports patchports helium release


prep:
	mkdir -p ${OBJPREFIX} ${TOPDIR}/dist ${BUILDROOT}

${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}: ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf/${BSDCONFIG}
	mkdir -p ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	(cd ${TOPDIR}/freebsd-src/sys/${MACHINE}/conf && config ${BSDCONFIG} \
	&& cd ../compile/${BSDCONFIG} && export MAKEOBJDIRPREFIX=${OBJPREFIX} \
	&& make depend)

checkout:
	test -d ${TOPDIR}/freebsd-src || \
		(cd ${TOPDIR} && git clone https://github.com/freebsd/freebsd-src.git && \
		cd freebsd-src && git checkout ${FREEBSD_BRANCH})

fetchports:
	mkdir -p ${TOPDIR}/portsnap
	portsnap -d ${TOPDIR}/portsnap -p ${TOPDIR}/ports auto

patchports:
	cd ${TOPDIR}/ports; \
		for patch in ${TOPDIR}/patches/ports-*.patch; do patch -p1 < $$patch; done; \


patchbsd: patches/[0-9]*.patch
	(cd ${TOPDIR}/freebsd-src && git checkout -f ${FREEBSD_BRANCH}; \
	git branch -D helium/12 || true; \
	git checkout -b helium/12; \
	for patch in ${TOPDIR}/patches/[0-9]*.patch; do patch -p1 < $$patch; done; \
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

# Utility target for port operations with consistent arguments
_portops: .PHONY
	make -C ${dir} _OSRELEASE=${OSRELEASE} PORTSDIR=${PORTSDIR} \
		NO_DEPENDS=1 LOCALBASE=/usr PREFIX=/usr BATCH=1 ${extra} ${tgt}

openjpeg: lcms2 png jpeg-turbo tiff
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch" _portops
	mkdir -p ${PORTSDIR}/graphics/${.TARGET}/work/build
	cd ${PORTSDIR}/graphics/${.TARGET}/work/build && cmake \
		-DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ../openjpeg-2.4.0
	make -C ${PORTSDIR}/graphics/${.TARGET}/work/build DESTDIR=${BUILDROOT} install

jpeg-turbo:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch" _portops
	mkdir -p ${PORTSDIR}/graphics/${.TARGET}/work/.build 
	cd ${PORTSDIR}/graphics/${.TARGET}/work/.build \
	&& cmake -DCMAKE_INSTALL_PREFIX=/usr ../libjpeg-turbo-2.0.6 \
	&& make
	make -C ${PORTSDIR}/graphics/${.TARGET}/work/.build DESTDIR=${BUILDROOT} install

lcms2:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make -C ${PORTSDIR}/graphics/${.TARGET}/work/lcms2-2.12 DESTDIR=${BUILDROOT} install

png:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make -C ${PORTSDIR}/graphics/${.TARGET}/work/libpng-1.6.37 DESTDIR=${BUILDROOT} install

tiff: jbigkit
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make -C ${PORTSDIR}/graphics/${.TARGET}/work/tiff-4.2.0 DESTDIR=${BUILDROOT} install

jbigkit:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

freetype2: brotli
	make dir=${PORTSDIR}/print/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/print/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

brotli:
	make dir=${PORTSDIR}/archivers/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/archivers/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

cairo: freetype2
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

fontconfig: expat2
	make dir=${PORTSDIR}/x11-fonts/${.TARGET} tgt="patch" _portops
	find ${PORTSDIR}/x11-fonts/${.TARGET}/work -name \*.py \
		-exec sed -ibak 's^/usr/bin/python^/usr/local/bin/python^g' \
		{} \;
	cd ${PORTSDIR}/x11-fonts/${.TARGET}/work/fontconfig-2.13.93 \
		&& ./configure --prefix=/usr --disable-docs \
		--sysconfdir=/etc --localstatedir=/var --disable-rpath --disable-iconv \
		--with-default-fonts=/usr/share/fonts --with-add-fonts=/usr/share/X11/fonts \
		&& sed -ibak 's/all-local: check-versions/all-local:/' Makefile \
		&& gmake && gmake DESTDIR=${BUILDROOT} install

expat2:
	make dir=${PORTSDIR}/textproc/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/textproc/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

libdrm:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

libffi:
	make dir=${PORTSDIR}/devel/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/devel/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

wayland: libffi
	make CFLAGS="$$CFLAGS -I/usr/local/include/libepoll-shim" dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

wayland-protocols:
	make CFLAGS="$$CFLAGS -I/usr/local/include/libepoll-shim" dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

mesa-dri: wayland wayland-protocols
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

mesa-libs:
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/graphics/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

xtrans:
	make dir=${PORTSDIR}/x11/${.TARGET} tgt="fetch patch build" _portops
	make dir=${PORTSDIR}/x11/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

xorg-server: xtrans
	make dir=${PORTSDIR}/x11-servers/${.TARGET} tgt="fetch patch build" \
		extra="CFLAGS=-UWITH_OPENSSL_BASE" _portops
	make dir=${PORTSDIR}/x11-servers/${.TARGET} tgt="do-install" extra=STAGEDIR=${BUILDROOT} _portops

packagesPreX: openjpeg freetype2 fontconfig libdrm mesa-dri mesa-libs
packagesPostX: cairo

xorgbuild: xorgmain xorgspecial xorg-server
xorgmain:
	mkdir -p xorg
	if [ ! -d xorg/util/modular ]; then \
		cd xorg \
		&& git clone git://anongit.freedesktop.org/git/xorg/util/modular util/modular; \
	fi
	mkdir -p xorg/_build
	cd xorg && export PREFIX=/usr LOCALSTATEDIR=/var MAKE=gmake \
		CONFFLAGS="--disable-docs --disable-specs --with-sdkdir=${BUILDROOT}/usr/include/xorg --with-xorg-conf-dir=${BUILDROOT}/usr/share/X11/xorg.conf.d --with-sysconfdir=/etc" \
		&& for mod in $$(cat ${TOPDIR}/xorg-modules.txt); do \
		./util/modular/build.sh -o $$mod --clone ${TOPDIR}/xorg/_build; done

xorgspecial:
	cd xorg && export PREFIX=/usr LOCALSTATEDIR=/var MAKE=gmake \
		CONFFLAGS="--disable-docs --disable-specs --with-sdkdir=${BUILDROOT}/usr/include/xorg --with-xorg-conf-dir=${BUILDROOT}/usr/share/X11/xorg.conf.d --with-sysconfdir=/etc" \
		PKG_CONFIG_PATH=/usr/libdata/pkgconfig:/usr/local/libdata/pkgconfig \
		CFLAGS="$$CFLAGS -I/usr/local/include -DERESTART=-1 -DETIME=ETIMEDOUT -DENODATA=ENOATTR" \
		LDFLAGS="$$LDFLAGS -L/usr/local/lib" \
		&& for mod in app/rendercheck app/xdriinfo; do \
		./util/modular/build.sh -o $$mod --clone ${TOPDIR}/xorg/_build; done

helium: extradirs mkfiles libobjc2 libunwind packagesPreX xorgbuild packagesPostX \
	frameworksclean frameworks copyfiles

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
	mv -f ${BUILDROOT}/usr/lib/pkgconfig/* \
		${BUILDROOT}/usr/libdata/pkgconfig
	rmdir ${BUILDROOT}/usr/lib/pkgconfig
	tar cJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/helium.txz .

desc_helium=Helium system
release: helium-package
	rm -f ${RLSDIR}/disc1.iso ${RLSDIR}/memstick.img 
	if [ -d ${RLSDIR}/disc1 ]; then \
		sudo chflags -R noschg,nouchg ${RLSDIR}/disc1 && sudo rm -rf ${RLSDIR}/disc1; \
	fi
	rm -f ${RLSDIR}/packagesystem
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; sudo -E \
		make -C ${TOPDIR}/freebsd-src/release \
		desc_helium="${desc_helium}" NOSRC=true NOPORTS=true \
		packagesystem disc1.iso memstick
	cp -fv ${RLSDIR}/disc1.iso ${RLSDIR}/memstick.img ${TOPDIR}/dist/
