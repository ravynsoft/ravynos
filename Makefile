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
PORTSDIR := ${TOPDIR}/ports
FONTSDIR := /System/Library/Fonts
CORES := 4

# List of packages that build without special handling
PKG_NORMAL_PRE_X= lang/python37 archivers/brotli archivers/zstd print/indexinfo \
	print/freetype2 textproc/libxml2 textproc/expat2 textproc/libxslt \
	devel/libffi devel/libudev-devd devel/gettext-runtime devel/libpciaccess \
	devel/libepoll-shim devel/libmtdev devel/libevdev devel/evdev-proto \
	devel/py-setuptools devel/py-pyudev devel/py-evdev devel/libgudev misc/pciids \
	converters/libiconv x11/xtrans x11/libxshmfence x11/libwacom x11/libinput \
	graphics/lcms2 graphics/png graphics/tiff graphics/jbigkit graphics/libdrm \
	graphics/mesa-dri graphics/mesa-libs graphics/libepoxy 

# Packages that need special build targets
PKG_SPECIAL_PRE_X= graphics/openjpeg graphics/jpeg-turbo graphics/wayland-protocols \
	graphics/wayland x11-fonts/fontconfig

# Same thing but built after Xorg
PKG_NORMAL_POST_X= graphics/cairo shells/zsh security/doas x11/xterm x11-fonts/freefont-ttf 
PKG_SPECIAL_POST_X=

PKG_NORMAL= ${PKG_NORMAL_PRE_X} ${PKG_NORMAL_POST_X}
PKG_SPECIAL= ${PKG_SPECIAL_PRE_X} ${PKG_SPECIAL_POST_X}
PKG_PRE_X= ${PKG_NORMAL_PRE_X} ${PKG_SPECIAL_PRE_X}
PKG_POST_X= ${PKG_NORMAL_POST_X} ${PKG_SPECIAL_POST_X}
PKG_ALL= ${PKG_NORMAL} ${PKG_SPECIAL} x11-servers/xorg-server

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

${TOPDIR}/ports:
	mkdir -p ${TOPDIR}/portsnap && \
		portsnap -d ${TOPDIR}/portsnap -p ${TOPDIR}/ports auto

${OBJPREFIX}/.patched_bsd: patches/[0-9]*.patch
	(cd ${TOPDIR}/freebsd-src && git checkout -f ${FREEBSD_BRANCH}; \
	git branch -D helium/12 || true; \
	git checkout -b helium/12; \
	for patch in ${TOPDIR}/patches/[0-9]*.patch; do patch -p1 < $$patch; done; \
	git commit -a -m "patched")
	touch ${OBJPREFIX}/.patched_bsd

${OBJPREFIX}/.patched_ports: patches/ports-*.patch ${TOPDIR}/ports
	cd ${TOPDIR}/ports && for patch in ${TOPDIR}/patches/ports-*.patch; \
		do patch -p1 < $$patch; done
	touch ${OBJPREFIX}/.patched_ports

freebsd: kernel base

kernel: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd ${TOPDIR}/freebsd-src/sys/${MACHINE}/compile/${BSDCONFIG}
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make ${MFLAGS} -C ${TOPDIR}/freebsd-src buildkernel 

base: ${TOPDIR}/freebsd-src ${OBJPREFIX}/.patched_bsd
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; make ${MFLAGS} -j${CORES} \
		-C ${TOPDIR}/freebsd-src buildworld

# Utility target for port operations with consistent arguments
_portops: .PHONY
	make -C ${PORTSDIR}/${dir} _OSRELEASE=${OSRELEASE} PORTSDIR=${PORTSDIR} \
	FONTSDIR=${FONTSDIR} DEFAULT_VERSIONS+=python3=3.7 DEFAULT_VERSIONS+=python=3.7 \
	STAGEDIR=${BUILDROOT} NO_DEPENDS=1 LOCALBASE=/usr PREFIX=/usr BATCH=1 ${extra} ${tgt}

# Utility target to create dummy packages for stuff now in the base OS [#40]
_makepkg: .PHONY
	export PORTINFO=$$(PORTSDIR=${PORTSDIR} ${PORTSDIR}/Tools/scripts/portsearch -p ${dir}); \
	export PORTNAME=$$(grep -E 'PORTNAME(\?=|=)' ${PORTSDIR}/${dir}/Makefile|cut -f2-); \
	VERSION=$$(echo $$PORTINFO|cut -f2-|cut -d' ' -f2|sed -e "s/$$PORTNAME-//") \
	INFO=$$(echo $$PORTINFO|sed -e 's/^.*Info: //'|cut -f2|sed -e 's/ Maint:.*//') \
	MAINT=$$(echo $$PORTINFO|sed -e 's/^.*Maint: //'|cut -f2|cut -d' ' -f1); \
	sed -e "s/%%NAME%%/$$PORTNAME/" -e "s/%%VERSION%%/$$VERSION/" -e "s@%%ORIGIN%%@${dir}@" \
	-e "s/%%COMMENT%%/$$INFO/" -e "s/%%MAINT%%/$$MAINT/" \
	<+MANIFEST.template >${OBJPREFIX}/+MANIFEST
	INSTALL_AS_USER=1 PKG_DBDIR=${BUILDROOT}/var/db/pkg \
		pkg register -M ${OBJPREFIX}/+MANIFEST 

makepkg: packages-db-clean
	for PKG in ${PKG_ALL}; do make dir=$$PKG _makepkg; done

graphics/openjpeg: graphics/lcms2 graphics/png graphics/jpeg-turbo graphics/tiff
	make dir=${.TARGET} tgt="fetch patch" _portops
	mkdir -p ${PORTSDIR}/${.TARGET}/work/build
	cd ${PORTSDIR}/${.TARGET}/work/build && cmake \
		-DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ../openjpeg-2.4.0
	make -C ${PORTSDIR}/${.TARGET}/work/build DESTDIR=${BUILDROOT} install

graphics/jpeg-turbo:
	make dir=${.TARGET} tgt="fetch patch" _portops
	mkdir -p ${PORTSDIR}/${.TARGET}/work/.build 
	cd ${PORTSDIR}/${.TARGET}/work/.build \
	&& cmake -DCMAKE_INSTALL_PREFIX=/usr ../libjpeg-turbo-2.0.6 \
	&& make
	make -C ${PORTSDIR}/${.TARGET}/work/.build DESTDIR=${BUILDROOT} install

graphics/{wayland,wayland-protocols}: devel/libffi
	make CFLAGS="$$CFLAGS -I/usr/include/libepoll-shim" dir=${.TARGET} tgt="fetch patch build do-install" _portops

graphics/mesa-dri: archivers/zstd graphics/wayland graphics/wayland-protocols
graphics/cairo: converters/libiconv textproc/libxslt devel/glib20 print/freetype2
graphics/tiff: graphics/jbigkit

devel/glib20:
	make dir=${.TARGET} tgt="fetch patch" _portops
	sed -ibak 's/http:\/\/.*docbook.xsl/\/usr\/local\/share\/xsl\/docbook\/manpages\/docbook.xsl/' ${PORTSDIR}/${.TARGET}/work/glib*/meson.build
	make dir=${.TARGET} tgt="build do-install" _portops

x11-servers/xorg-server: x11/xtrans
	make FONTPATH_ROOT=${FONTSDIR} dir=${.TARGET} tgt="clean fetch patch build do-install" \
		extra="CFLAGS=-UWITH_OPENSSL_BASE" _portops

x11-fonts/fontconfig: textproc/expat2
	make dir=${.TARGET} tgt="patch" _portops
	find ${PORTSDIR}/${.TARGET}/work -name \*.py \
		-exec sed -ibak 's^/usr/bin/python^/usr/local/bin/python^g' \
		{} \;
	cd ${PORTSDIR}/${.TARGET}/work/fontconfig-2.13.93 \
		&& ./configure --prefix=/usr --disable-docs \
		--sysconfdir=/etc --localstatedir=/var --disable-rpath --disable-iconv \
		--with-default-fonts=${FONTSDIR} --with-add-fonts=/usr/share/fonts \
		&& sed -ibak 's/all-local: check-versions/all-local:/' Makefile \
		&& gmake && gmake DESTDIR=${BUILDROOT} install

packagesPreX: ${OBJPREFIX}/.patched_ports ${PKG_PRE_X}
packagesPostX: ${PKG_POST_X} packages-postbuild 

${PKG_NORMAL}:
	PYTHONPATH=/usr/local/lib/python3.7/site-packages \
	CFLAGS="$$CFLAGS -I/usr/include/libdrm" \
	make dir=${.TARGET} tgt="fetch patch build do-install" _portops

packages-postbuild:
	ln -sf python3.7 ${BUILDROOT}/usr/bin/python3
	ln -sf doas ${BUILDROOT}/usr/bin/sudo

packages-clean:
	for pkg in ${PKG_ALL}; do \
		make dir=$$pkg tgt="clean" _portops; \
		rm -rf ${PORTSDIR}/$$pkg/work
	done

packages-db-clean:
	rm -f ${BUILDROOT}/var/db/pkg/*

xorgbuild: fetchxorg xorgmain1 x11-servers/xorg-server xorgmain2 xorgspecial xf86-input-libinput xorgpostbuild

xorgpostbuild:
	sudo chmod u+s ${BUILDROOT}/usr/bin/Xorg.wrap
	tar -C ${BUILDROOT}/${BUILDROOT}/usr/local -cf - share | tar -C ${BUILDROOT}/usr -xf -
	tar -C ${BUILDROOT}/${BUILDROOT}/usr -cf - share | tar -C ${BUILDROOT}/usr -xf -
	tar -C ${BUILDROOT}/${BUILDROOT}/usr -cf - include | tar -C ${BUILDROOT}/usr -xf -
	_br=${BUILDROOT}; _tail="$${_br#/*/}"; _head="$${_br%/$$_tail}"; rm -rf ${BUILDROOT}$${_head}
	if [ ! -L ${BUILDROOT}/usr/etc ]; then \
		tar -C ${BUILDROOT}/usr -cf - etc | tar -C ${BUILDROOT} -xf -; \
		rm -rf ${BUILDROOT}/usr/etc; \
		ln -sf ../etc ${BUILDROOT}/usr/etc; \
	fi 
	tar -C ${BUILDROOT}/usr/share -cf - pkgconfig | tar -C ${BUILDROOT}/usr/libdata -xf -
	rm -rf ${BUILDROOT}/usr/share/pkgconfig
	mkdir -p ${BUILDROOT}/Users

fetchxorg:
	mkdir -p xorg
	if [ ! -d xorg/util/modular ]; then \
		cd xorg \
		&& git clone git://anongit.freedesktop.org/git/xorg/util/modular util/modular; \
	fi

xorgmain1:
	cd xorg && export PREFIX=/usr LOCALSTATEDIR=/var MAKE=gmake DESTDIR=${BUILDROOT} \
		PKG_CONFIG_SYSROOT_DIR=${BUILDROOT} \
		CONFFLAGS="--disable-docs --disable-specs --with-sysconfdir=/etc --with-appdefaultdir=/usr/share/X11/app-defaults" \
		CFLAGS="-I/usr/local/include" \
		LDFLAGS="-L/usr/local/lib" \
		&& for mod in $$(cat ${TOPDIR}/xorg-modules1.txt); do \
		./util/modular/build.sh -o $$mod --clone /usr; done

xorgmain2:
	cd xorg && export PREFIX=/usr LOCALSTATEDIR=/var MAKE=gmake DESTDIR=${BUILDROOT} \
		PKG_CONFIG_SYSROOT_DIR=${BUILDROOT} \
		PKG_CONFIG_PATH=${BUILDROOT}/usr/libdata/pkgconfig:/usr/libdata/pkgconfig:/usr/local/libdata/pkgconfig \
		CONFFLAGS="--disable-docs --disable-specs --with-sysconfdir=/etc --with-fontrootdir=${FONTSDIR}" \
		CFLAGS="-I/usr/local/include -I${BUILDROOT}/usr/include/xorg -I${BUILDROOT}/usr/include/libdrm -I${BUILDROOT}/usr/include/pixman-1" \
		LDFLAGS="-L/usr/local/lib -L${BUILDROOT}/usr/lib" \
		&& for mod in $$(cat ${TOPDIR}/xorg-modules2.txt); do \
		./util/modular/build.sh -o $$mod --clone /usr; done


xorgspecial:
	cd xorg && export PREFIX=/usr LOCALSTATEDIR=/var MAKE=gmake \
		PKG_CONFIG_PATH=/usr/libdata/pkgconfig:/usr/local/libdata/pkgconfig \
		CFLAGS="-I/usr/local/include -DERESTART=-1 -DETIME=ETIMEDOUT -DENODATA=ENOATTR" \
		LDFLAGS="-L/usr/local/lib" \
		&& for mod in app/rendercheck app/xdriinfo; do \
		./util/modular/build.sh -o $$mod --clone ${BUILDROOT}/usr; done

xorg/driver/xf86-input-libinput:
	cd xorg/driver && git clone https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput

xf86-input-libinput: xorg/driver/xf86-input-libinput devel/libevdev \
	devel/libgudev x11/libwacom devel/libmtdev x11/libinput devel/py-setuptools devel/py-pyudev \
	devel/py-evdev
	cd xorg/driver/xf86-input-libinput \
	&& ACLOCAL_PATH=/usr/share/aclocal:/usr/local/share/aclocal autoreconf -vif \
	&& ./configure --prefix=/usr --disable-docs --with-sysconfdir=/etc \
	&& make && make DESTDIR=${BUILDROOT} install
	cp -f xorg/driver/xf86-input-libinput/conf/40-libinput.conf ${BUILDROOT}/usr/share/X11/xorg.conf.d/

helium: extradirs mkfiles libobjc2 libunwind packagesPreX xorgbuild packagesPostX \
	frameworksclean frameworks copyfiles symlink-fonts makepkg

symlink-fonts:
	for FONT in SourceCodePro dejavu font-awesome wqy; do \
	ln -sf /usr/local/share/fonts/$$FONT ${BUILDROOT}/System/Library/Fonts; done

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
	mv -f ${BUILDROOT}/usr/lib/pkgconfig/* \
		${BUILDROOT}/usr/libdata/pkgconfig || true
	rmdir ${BUILDROOT}/usr/lib/pkgconfig || true
	tar cJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${RLSDIR}/helium.txz .

${TOPDIR}/ISO:
	git clone https://github.com/mszoek/ISO.git
	cd ISO && git checkout helium

desc_helium=Helium system
release: helium-package ${TOPDIR}/ISO
	rm -f ${RLSDIR}/packagesystem
	export MAKEOBJDIRPREFIX=${OBJPREFIX}; sudo \
		make -C ${TOPDIR}/freebsd-src/release \
		desc_helium="${desc_helium}" NOSRC=true NOPORTS=true packagesystem 
	cd ISO && workdir=${OBJPREFIX} HELIUM=${TOPDIR} sudo ./build.sh hello Helium_${HELIUM_VERSION}
