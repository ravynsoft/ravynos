#!/bin/sh
# Fix various ports to build correctly as Airyx base OS
# This script is a Hideous Hack to get things building quickly. It should be
# replaced (as much as possible) with tuning build options and other
# supported techniques that help ensure consistent settings across ports

#-------------------------------------------------------------------------
#   GLOBAL CHANGES
#-------------------------------------------------------------------------

# Make sure everything uses Airyx paths
sed -i_ -e 's@${LOCALBASE}/etc/gnome.subr@/etc/gnome.subr@' /usr/ports/Mk/Uses/gnome.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@' /usr/ports/Mk/Uses/display.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/Mk/Uses/xorg-cat.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/Mk/Uses/fonts.mk
sed -i_ -e 's@--mandir man@--mandir share/man@' /usr/ports/Mk/Uses/meson.mk
sed -i_ -e 's@${PREFIX}/etc/@/etc/@' -e 's@ETCDIR_REL.*$@&\nMANPREFIX_REL?=\t${MANPREFIX:S,^${PREFIX}/,,}\nPLIST_SUB+=\tMANPREFIX="${MANPREFIX_REL}"\n@' /usr/ports/Mk/bsd.port.mk

# Fake out the SSL detection stuff which doesn't work with LOCALBASE and PREFIX set to /usr
sed -i_ -e '66,83s/^./#&/' /usr/ports/Mk/Uses/ssl.mk

# Prevent reordering /usr/include to the end of system includes - it breaks C++
sed -i_ -e 's/^CPPFLAGS/# CPPFLAGS/' -e 's/^CFLAGS/# CFLAGS/' -e 's/^CXXFLAGS/# CXXFLAGS/' /usr/ports/Mk/Uses/localbase.mk
sed -i_ -e 's@${QT_LIBDIR} \\@${QT_LIBDIR}@' -e 's@^.*--with-extra-includes.*$@@' -e 's@^.*--with-extra-libs.*$@@' /usr/ports/Mk/Uses/qmake.mk
sed -i_ -e 's@^CONFIGURE_ARGS@# &@' /usr/ports/Mk/Uses/xorg.mk

# Pre-emptively patch all pkg-plists with the right ETCDIR, MANPREFIX and FONTSDIR
find /usr/ports -name pkg-plist -exec sed -i_ -e 's@etc/@/etc/@' -e 's@//etc@/etc@' -e 's@^man@%%MANPREFIX%%/man@' -e 's@^share/man@%%MANPREFIX%%/man@' -e 's@share/fonts@/System/Library/Fonts@' -e 's@ share/fonts@ /System/Library/Fonts@' {} \;

#-------------------------------------------------------------------------
#   INDIVIDUAL PORT CHANGES
#-------------------------------------------------------------------------

# Perl 5.32
sed -i_ -e 's/${PREFIX}\/etc/\/etc/' /usr/ports/lang/perl5.32/Makefile
sed -i_ -e 's/^\t*${INSTALL_DATA} ${WRKDIR}.*$/\t${MKDIR} -p ${STAGEDIR}\/etc\/man.d ${STAGEDIR}\/usr\/libdata\/ldconfig\n&/' /usr/ports/lang/perl5.32/Makefile

# help2man
sed -i_ -e 's@man/man1@%%MANPREFIX%%/man/man1@' -e 's@man/${lang}@%%MANPREFIX%%/man/${lang}@' /usr/ports/misc/help2man/Makefile

# Undo meson patch that breaks mandir
rm -f /usr/ports/devel/meson/files/patch-setup.py

# ca_root_nss, plasma5-kinfocenter, smartmontools, mesa-libs, p11-kit, fontconfig
sed -i_ -e 's@${PREFIX}/etc@/etc@g' /usr/ports/security/ca_root_nss/Makefile /usr/ports/sysutils/plasma5-kinfocenter/Makefile /usr/ports/sysutils/smartmontools/Makefile /usr/ports/security/p11-kit/Makefile /usr/ports/graphics/mesa-libs/Makefile /usr/ports/x11-fonts/fontconfig/Makefile
sed -i_ -e 's@# Fallback.*$@&\n\t${MKDIR} -p ${STAGEDIR}/etc/libmap.d@' /usr/ports/graphics/mesa-libs/Makefile
# rhash
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/security/rhash/Makefile

# libarchive
sed -i_ -e 's@man/@${MANPREFIX}/man/@' /usr/ports/archivers/libarchive/Makefile

# cmake
sed -i_ -e 's@MANPAGES%%man@MANPAGES%%%%MANPREFIX%%/man@' /usr/ports/devel/cmake/pkg-plist
sed -i_ -e 's@--prefix=@--mandir=${MANPREFIX_REL}/man &@' /usr/ports/devel/cmake/Makefile

# libedit
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@g' /usr/ports/devel/libedit/Makefile

#lua52
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@g' -e 's@INSTALL_EXEC=@INSTALL_MAN=${STAGEDIR}${MANPREFIX}/man/man1 &@' /usr/ports/lang/lua52/Makefile

# pcre
sed -i_ -e 's@CONFIGURE_ARGS=@& --mandir=${MANPREFIX}/man@' /usr/ports/devel/pcre/Makefile
sed -i_ -e 's@MAN3%%@MANPREFIX%%/@' /usr/ports/devel/pcre/pkg-plist

# xmlcatmgr
sed -i_ -e 's@^\.include <bsd\.port\.mk>@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/xml ${STAGEDIR}${PREFIX}/share/sgml\n&@' /usr/ports/textproc/xmlcatmgr/Makefile

# qt5-core
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/qt5-core/Makefile

# Fontconfig, font-alias, luit
sed -i_ -e 's@^FCDEFAULTFONTS.*@FCDEFAULTFONTS= /System/Library/Fonts /Library/Fonts@' -e 's@post-patch:@&\n\t${REINPLACE_CMD} -e "s,%%STAGEDIR%%,${STAGEDIR},g" ${PATCH_WRKSRC}/conf.d/link_confs.py@' /usr/ports/x11-fonts/fontconfig/Makefile
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@' -e 's@man/man1@${MANPREFIX}/man/man1@g' /usr/ports/x11/luit/Makefile

# LLVM10
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm10/Makefile

# dbus
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/devel/dbus/Makefile

# qt5-widgets
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps\n&@' /usr/ports/x11-toolkits/qt5-widgets/Makefile

# desktop-file-utils
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/applications\n&@' /usr/ports/devel/desktop-file-utils/Makefile

# aspell
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/textproc/aspell/Makefile

# gnome_subr
sed -i_ -e 's@etc/@/etc/@' -e 's@${PREFIX}/etc@/etc@' -e 's@^do-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/sysutils/gnome_subr/Makefile


# tpm-emulator
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/emulators/tpm-emulator/Makefile

# trousers
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d@' /usr/ports/security/trousers/Makefile

# libpaper
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' -e 's@${PREFIX}/etc@/etc@' /usr/ports/print/libpaper/Makefile

# cups
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@${LOCALBASE}/etc@/etc@g' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}/etc/pam.d ${STAGEDIR}/etc/devd@' /usr/ports/print/cups/Makefile

# jbigkit
sed -i_ -e 's@^man/@%%MANPREFIX%%/man/@' /usr/ports/graphics/jbigkit/pkg-plist

# encodings
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@' /usr/ports/x11-fonts/encodings/Makefile

# pango
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/x11-toolkits/pango/Makefile

# qt5-assistant, qt5-designer
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps/ ${STAGEDIR}/usr/share/applications/@' /usr/ports/devel/qt5-assistant/Makefile /usr/ports/devel/qt5-designer/Makefile

# gstreamer1
sed -i_ -e 's@usr/local@usr@' /usr/ports/multimedia/gstreamer1/pkg-plist

# libvdpau, alsa-lib
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc\n&@' /usr/ports/audio/alsa-lib/Makefile

# plasma5-plasma-workspace, plasma5-kinfocenter, akonadi, plasma5-ksysguard, plasma5-plasma-desktop, smartmontools
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/sysutils/smartmontools/Makefile

# signon-plugin-oauth2
sed -i_ -e 's@^post-patch:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-plugin-oauth2/Makefile

# signon-kwallet-extension
sed -i_ -e 's@^\.include@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-kwallet-extension/Makefile

# mysql57-server
sed -i_ -e 's@DIR="etc@DIR="/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}${PREFIX}/libdata/ldconfig@' /usr/ports/databases/mysql57-server/Makefile

# graphviz, mysql57-client
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig/\n&@' /usr/ports/graphics/graphviz/Makefile /usr/ports/databases/mysql57-client/Makefile

# llvm90
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm90/Makefile

# qt5-webengine
sed -i_ -e 's@^.*C_INCLUDE_PATH=.*$@\\@' -e 's@^.*CPLUS_INCLUDE_PATH=.*$@\\@' /usr/ports/www/qt5-webengine/Makefile

# webcamd
sed -i_ -e 's@^post-install:@pre-install:\n\tmkdir -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/multimedia/webcamd/Makefile

# xterm
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/share/applications\n&@' /usr/ports/x11/xterm/Makefile

# Boatloads of x11 stuff
sed -i_ -e 's@ man/@ ${MANPREFIX}/man/@' /usr/ports/x11/{setxkbmap,smproxy,xcursorgen,appres,xf86dga,iceauth,sessreg,xauth,xbacklight,xcmsdb,xdpyinfo,xdriinfo,xev,xgamma,xhost,xinput,xkbevd,xlsatoms,xlsclients,xmodmap,xprop,xrdb,xrefresh,xset,xsetroot,xvinfo,xwd,xwininfo,xwud}/Makefile
sed -i_ -e 's@ man/@ ${MANPREFIX}/man/@' /usr/ports/x11-fonts/bdftopcf/Makefile
sed -i_ -e 's@man/man1@${MANPREFIX}/man/man1@' /usr/ports/x11/{xrandr,xkbcomp}/Makefile
sed -i_ -e 's@man/man1@${MANPREFIX}/man/man1@g' /usr/ports/x11/xpr/Makefile
sed -i_ -e 's@^post-patch:@CONFIGURE_ARGS+= --sysconfdir=/etc\n&@' /usr/ports/x11/xinit/Makefile
# mkfontscale
sed -i_ -e 's@^\t\tman/@\t\t${MANPREFIX}/man/@' /usr/ports/x11-fonts/mkfontscale/Makefile

# freetype2
sed -i_ -e 's@%%man/@%%%%MANPREFIX%%/man/@' /usr/ports/print/freetype2/pkg-plist


#------------------------
# DEPRECATED - REMOVE ME
#-------------------------

# psutils
#sed -i_ -e 's@MANDIR ?=@MANDIR =@' -e 's@$(MANPREFIX)/man@$(MANPREFIX)/share/man@' /usr/ports/print/psutils/files/patch-Makefile.unix
#rm -f /usr/ports/print/psutils/files/patch-Makefile.unix_
#sed -i_ -e 's@^man/@share/man/@' -e 's@\.gz$@@' /usr/ports/print/psutils/pkg-plist
