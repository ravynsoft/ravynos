# Primary makefile for the Airyx OS

TOPDIR := ${.CURDIR}
OBJPREFIX := ${HOME}/obj.${MACHINE}
BUILDROOT := ${OBJPREFIX}/buildroot
PORTSROOT := ${OBJPREFIX}/portsroot
AIRYX_VERSION != head -1 ${TOPDIR}/version.txt
AIRYX_CODENAME != tail -1 ${TOPDIR}/version.txt
MKINCDIR := -m/usr/share/mk -m${TOPDIR}/mk
CORES != sysctl -n hw.ncpu
SUDO != test "$$USER" == "root" && echo "" || echo "sudo"

.export TOPDIR OBJPREFIX BUILDROOT PORTSROOT AIRYX_VERSION AIRYX_CODENAME MKINCDIR CORES SUDO

# We need the installed frameworks to correctly link CoreServices and applications
airyx: airyxbase installairyx sysmenu coreservices bin

airyxbase: mkfiles frameworks copyfiles
	tar -C ${BUILDROOT}/usr/lib -cpf pkgconfig | tar -C ${BUILDROOT}/usr/share -xpf -
	rm -rf ${BUILDROOT}/usr/lib/pkgconfig

bin: .PHONY
	${MAKE} -C bin all install

jdk: .PHONY
	${MAKE} -C Library/Java all

sysmenu: .PHONY
	mkdir -p ${OBJPREFIX}/sysmenu
	echo '#define AIRYX_VERSION "${AIRYX_VERSION}"' > ${TOPDIR}/sysmenu/src/version.h
	echo '#define AIRYX_CODENAME "${AIRYX_CODENAME}"' >> ${TOPDIR}/sysmenu/src/version.h
	cmake -S ${TOPDIR}/sysmenu -B ${OBJPREFIX}/sysmenu
	make -C ${OBJPREFIX}/sysmenu DESTDIR=${BUILDROOT} install

frameworks:
	${MAKE} -C Frameworks all

coreservices:
	${MAKE} -C CoreServices all

airyx-package:
	${SUDO} mkdir -p ${TOPDIR}/dist
	${SUDO} tar cvJ -C ${BUILDROOT} --gid 0 --uid 0 -f ${TOPDIR}/dist/airyx.txz .

installairyx: airyx-package
	${SUDO} tar -C / -xvf ${TOPDIR}/dist/airyx.txz

prep: cleanroot
	mkdir -p ${OBJPREFIX} ${TOPDIR}/dist ${BUILDROOT}
	mkdir -p ${BUILDROOT}/etc ${BUILDROOT}/var/run ${BUILDROOT}/usr/sbin
	${SUDO} cp -f ${TOPDIR}/make.conf ${TOPDIR}/resolv.conf ${BUILDROOT}/etc/
	for x in System System/Library/Frameworks Library Users Applications Volumes; \
		do mkdir -p ${BUILDROOT}/$$x; \
	done

cleanroot:
	if [ -d ${BUILDROOT} ]; then \
		${SUDO} chflags -R noschg,nouchg ${BUILDROOT}; \
		${SUDO} rm -rf ${BUILDROOT}; \
	fi

getports:
	${SUDO} git clone https://git.freebsd.org/ports.git -b2021Q3 /usr/ports
	${SUDO} ${TOPDIR}/Tools/patch-ports.sh
	${SUDO} cp -f ${TOPDIR}/patches/patch-conf.d_link__confs.py /usr/ports/x11-fonts/fontconfig/files/
	${SUDO} mkdir -p /usr/ports/graphics/jpeg-turbo/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-cmakescripts_GNUInstallDirs.cmake /usr/ports/graphics/jpeg-turbo/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-meson.build /usr/ports/sysutils/polkit/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-freebsd_Makefile /usr/ports/shells/bash-completion/files/
	${SUDO} mkdir -p /usr/ports/sysutils/bsdisks/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-CMakeLists.txt /usr/ports/sysutils/bsdisks/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-mysql57_install__layout.cmake /usr/ports/databases/mysql57-client/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-webcamd-Makefile /usr/ports/multimedia/webcamd/files/
	${SUDO} mkdir -p /usr/ports/audio/lilv/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-waflib_extras_autowaf.py /usr/ports/audio/lilv/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-libsysinfo_Makefile /usr/ports/devel/libsysinfo/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-libtaskmanager_xwindowtasksmodel.cpp /usr/ports/x11/plasma5-plasma-workspace/files/
	${SUDO} cp -f ${TOPDIR}/patches/patch-airyx-boldmenus /usr/ports/x11/plasma5-plasma-workspace/files/
	${SUDO} mkdir -p /usr/ports/x11-toolkits/kf5-kxmlgui/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-src_ktoolbarhandler.cpp ${TOPDIR}/patches/patch-src_ui__standards.rc /usr/ports/x11-toolkits/kf5-kxmlgui/files/
	${SUDO} mkdir -p /usr/ports/devel/kf5-ktexteditor/files
	${SUDO} cp -f ${TOPDIR}/patches/patch-src_data_katepart5ui.rc /usr/ports/devel/kf5-ktexteditor/files/
	${SUDO} mkdir -p /usr/ports/distfiles

# Prepare the chroot jail for our ports builds
prepports:
	if [ -d ${PORTSROOT} ]; then \
		${SUDO} chflags -R noschg,nouchg ${PORTSROOT}; \
		${SUDO} rm -rf ${PORTSROOT}; \
	fi
	mkdir -p ${PORTSROOT}/etc ${PORTSROOT}/var/run ${PORTSROOT}/usr/sbin
	${SUDO} cp -f ${TOPDIR}/make.conf ${TOPDIR}/resolv.conf ${PORTSROOT}/etc/
	${SUDO} cp -f /var/run/ld-elf.so.hints ${PORTSROOT}/var/run
	${SUDO} cp -f /usr/sbin/pkg-static ${PORTSROOT}/usr/sbin || true
	if [ ! -f ${PORTSROOT}/usr/sbin/pkg-static ]; then ${SUDO} cp ${PORTSROOT}/usr/sbin/pkg ${PORTSROOT}/usr/sbin/pkg-static; fi
	if [ ! -f ${TOPDIR}/dist/base.txz ]; then fetch -o ${TOPDIR}/dist/base.txz https://dl.cloudsmith.io/public/airyx/core/raw/files/base.txz; fi
	${SUDO} tar xvf ${TOPDIR}/dist/base.txz -C ${PORTSROOT}
	${SUDO} ln -s libncurses.so ${PORTSROOT}/usr/lib/libncurses.so.6
	${SUDO} ln -s python3.8 ${PORTSROOT}/usr/bin/python # for nodejs build

/usr/ports/{archivers,audio,databases,devel,dns,editors,emulators,graphics,lang,misc,multimedia,net,ports-mgmt,security,shells,sysutils,textproc,www,x11,x11-drivers,x11-fonts,x11-fm,x11-themes,x11-toolkits}/*: .PHONY
	${SUDO} ${MAKE} -C ${.TARGET} DESTDIR=${PORTSROOT} install

mountsrc:
	${SUDO} mount_nullfs ${TOPDIR}/../airyx-freebsd/ ${PORTSROOT}/usr/src

umountsrc:
	${SUDO} umount ${PORTSROOT}/usr/src

zsh: /usr/ports/shells/zsh
	${SUDO} ln -f ${PORTSROOT}/usr/bin/zsh ${PORTSROOT}/bin/zsh

plasma: /usr/ports/x11/plasma5-plasma /usr/ports/x11/konsole /usr/ports/x11-fm/dolphin
xorg: /usr/ports/x11/xorg /usr/ports/x11-themes/adwaita-icon-theme /usr/ports/devel/desktop-file-utils
misc: /usr/ports/archivers/brotli /usr/ports/graphics/argyllcms /usr/ports/multimedia/gstreamer1-plugins-all
misc2: /usr/ports/x11/zenity /usr/ports/sysutils/cpdup /usr/ports/audio/freedesktop-sound-theme /usr/ports/sysutils/fusefs-libs mountsrc /usr/ports/graphics/gpu-firmware-kmod /usr/ports/sysutils/iichid /usr/ports/net/libdnet /usr/ports/archivers/libmspack /usr/ports/security/libretls /usr/ports/devel/libsigc++20 /usr/ports/multimedia/libva-intel-driver /usr/ports/dns/nss_mdns /usr/ports/emulators/open-vm-tools /usr/ports/net/openntpd /usr/ports/sysutils/pv /usr/ports/misc/usbids /usr/ports/misc/utouch-kmod umountsrc /usr/ports/net/wpa_supplicant_gui /usr/ports/devel/xdg-user-dirs
misc3: /usr/ports/security/sudo /usr/ports/devel/libqtxdg /usr/ports/devel/git /usr/ports/x11/slim /usr/ports/lang/python3 /usr/ports/x11-toolkits/py-qt5-widgets /usr/ports/www/py-qt5-webengine /usr/ports/misc/qt5-l10n /usr/ports/www/py-beautifulsoup /usr/ports/devel/py-dateutil /usr/ports/sysutils/py-psutil /usr/ports/devel/py-qt5-dbus /usr/ports/databases/sqlite3 /usr/ports/devel/py-xmltodict /usr/ports/devel/py-pip /usr/ports/x11-fonts/font-awesome /usr/ports/sysutils/dmidecode /usr/ports/ports-mgmt/pkg /usr/ports/x11/libfm-qt /usr/ports/x11/libfm /usr/ports/graphics/mesa-gallium-xa /usr/ports/x11-drivers/xf86-video-intel /usr/ports/x11-drivers/xf86-video-vmware /usr/ports/x11-fonts/sourcecodepro-ttf /usr/ports/x11-fonts/wqy /usr/ports/databases/py-sqlite3 /usr/ports/multimedia/py-qt5-multimedia /usr/ports/archivers/zip /usr/ports/devel/kf5-ktexteditor
misc4: /usr/ports/x11-drivers/xf86-input-evdev /usr/ports/x11-drivers/xf86-input-synaptics /usr/ports/x11-drivers/xf86-input-vmmouse /usr/ports/x11-drivers/xf86-video-vmware /usr/ports/x11-drivers/xf86-video-nv /usr/ports/x11-drivers/xf86-video-intel /usr/ports/x11-drivers/xf86-video-ati /usr/ports/x11-drivers/xf86-video-cirrus /usr/ports/www/node /usr/ports/devel/libdbusmenu
buildports: zsh xorg plasma misc misc2 misc3 misc4

makepackages:
	${SUDO} rm -rf /usr/ports/packages
	${SUDO} mkdir -p /usr/ports/packages
	${SUDO} mount_nullfs /usr/ports/packages ${PORTSROOT}/mnt
	${SUDO} chroot ${PORTSROOT} /bin/sh -c '/usr/sbin/pkg-static create -a -o /mnt'
	${SUDO} umount ${PORTSROOT}/mnt
	${SUDO} pkg repo -o /usr/ports/packages /usr/ports/packages

copyfiles:
	cp -fvR ${TOPDIR}/etc ${BUILDROOT}
	sed -i_ -e "s/__VERSION__/${AIRYX_VERSION}/" -e "s/__CODENAME__/${AIRYX_CODENAME}/" ${BUILDROOT}/etc/motd
	rm -f ${BUILDROOT}/etc/motd_
	${MAKE} -C Colors

mkfiles:
	mkdir -p ${BUILDROOT}/usr/share/mk
	cp -fv ${TOPDIR}/mk/*.mk ${BUILDROOT}/usr/share/mk/


${TOPDIR}/ISO:
	cd ${TOPDIR} && git clone https://github.com/mszoek/ISO.git
	cd ${TOPDIR}/ISO && git checkout airyx

${TOPDIR}/dist/CocoaDemo.app.txz:
	${MAKE} -C ${TOPDIR}/examples/app clean
	${MAKE} -C ${TOPDIR}/examples/app 
	tar -C ${TOPDIR}/examples/app -cf ${.TARGET} CocoaDemo.app

iso:
	cp -f ${TOPDIR}/version.txt ${TOPDIR}/ISO/overlays/ramdisk/version
	cd ${TOPDIR}/ISO && workdir=${OBJPREFIX} AIRYX=${TOPDIR} ${SUDO} -E ./build.sh airyx Airyx_${AIRYX_VERSION}

release: airyx-package ${TOPDIR}/ISO ${TOPDIR}/dist/CocoaDemo.app.txz iso

