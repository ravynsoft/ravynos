#!/bin/sh
# Fix various ports to build correctly as Airyx base OS
# This script is a Hideous Hack to get things building quickly. It should be
# replaced (as much as possible) with tuning build options and other
# supported techniques that help ensure consistent settings across ports

#-------------------------------------------------------------------------
#   GLOBAL CHANGES
#-------------------------------------------------------------------------

# Make sure everything uses Airyx paths
sed -i_ -e 's@${KDE_PREFIX}/man@${KDE_PREFIX}/share/man@g' /usr/ports/Mk/Uses/kde.mk
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

# Undo meson patch that breaks mandir
rm -f /usr/ports/devel/meson/files/patch-setup.py

sed -i_ -e 's/${PREFIX}\/etc/\/etc/' /usr/ports/lang/perl5.32/Makefile
sed -i_ -e 's/^\t*${INSTALL_DATA} ${WRKDIR}.*$/\t${MKDIR} -p ${STAGEDIR}\/etc\/man.d ${STAGEDIR}\/usr\/libdata\/ldconfig\n&/' /usr/ports/lang/perl5.32/Makefile
sed -i_ -e 's@man/man1@%%MANPREFIX%%/man/man1@' -e 's@man/${lang}@%%MANPREFIX%%/man/${lang}@' /usr/ports/misc/help2man/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' /usr/ports/security/ca_root_nss/Makefile /usr/ports/sysutils/plasma5-kinfocenter/Makefile /usr/ports/sysutils/smartmontools/Makefile /usr/ports/security/p11-kit/Makefile /usr/ports/graphics/mesa-libs/Makefile /usr/ports/x11-fonts/fontconfig/Makefile 
sed -i_ -e 's@# Fallback.*$@&\n\t${MKDIR} -p ${STAGEDIR}/etc/libmap.d@' /usr/ports/graphics/mesa-libs/Makefile
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/security/rhash/Makefile /usr/ports/sysutils/consolekit2/Makefile /usr/ports/devel/dbus-glib/Makefile
sed -i_ -e 's@CONFIGURE_ARGS?=@& --sysconfdir=/etc @' -e 's@${PREFIX}/etc@/etc@g' /usr/ports/net/avahi-app/Makefile
sed -i_ -e 's@man/@${MANPREFIX}/man/@' /usr/ports/archivers/libarchive/Makefile
sed -i_ -e 's@MANPAGES%%man@MANPAGES%%%%MANPREFIX%%/man@' /usr/ports/devel/cmake/pkg-plist /usr/ports/devel/kf5-extra-cmake-modules/pkg-plist /usr/ports/graphics/openjpeg15/pkg-plist
sed -i_ -e 's@ICC%%man@ICC%%%%MANPREFIX%%/man@' /usr/ports/graphics/lcms2/pkg-plist
sed -i_ -e 's@--prefix=@--mandir=${MANPREFIX_REL}/man &@' /usr/ports/devel/cmake/Makefile
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@g' /usr/ports/devel/libedit/Makefile /usr/ports/graphics/lcms2/Makefile
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@g' -e 's@INSTALL_EXEC=@INSTALL_MAN=${STAGEDIR}${MANPREFIX}/man/man1 &@' /usr/ports/lang/lua53/Makefile /usr/ports/lang/lua52/Makefile
sed -i_ -e 's@CONFIGURE_ARGS=@& --mandir=${MANPREFIX}/man@' /usr/ports/devel/pcre/Makefile
sed -i_ -e 's@MAN3%%@MANPREFIX%%/@' -e 's@%%DANE%%man@%%DANE%%%%MANPREFIX%%/man@' /usr/ports/devel/pcre/pkg-plist /usr/ports/security/gnutls/pkg-plist 
sed -i_ -e 's@^\.include <bsd\.port\.mk>@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/xml ${STAGEDIR}${PREFIX}/share/sgml\n&@' -e 's@ man/@ ${MANPREFIX}/man/@g' /usr/ports/textproc/xmlcatmgr/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/qt5-core/Makefile
sed -i_ -e 's@^FCDEFAULTFONTS.*@FCDEFAULTFONTS= /System/Library/Fonts</dir> <dir>/Library/Fonts</dir> <dir>~/Library/Fonts@' -e 's@post-patch:@&\n\t${REINPLACE_CMD} -e "s,%%STAGEDIR%%,${STAGEDIR},g" ${PATCH_WRKSRC}/conf.d/link_confs.py@' /usr/ports/x11-fonts/fontconfig/Makefile
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@' -e 's@man/man1@${MANPREFIX}/man/man1@g' /usr/ports/x11/luit/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm12/Makefile /usr/ports/devel/llvm10/Makefile
sed -i_ -e 's@^post-stage:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/lang/gcc10/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' -e's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' /usr/ports/devel/dbus/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps\n&@' /usr/ports/x11-toolkits/qt5-widgets/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/applications\n&@' /usr/ports/devel/desktop-file-utils/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/textproc/aspell/Makefile
sed -i_ -e 's@etc/@/etc/@' -e 's@${PREFIX}/etc@/etc@' -e 's@^do-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/sysutils/gnome_subr/Makefile
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' -e 's@${PREFIX}/var@/var@g' /usr/ports/emulators/tpm-emulator/Makefile
sed -i_ -e 's@ var@ /var@' /usr/ports/emulators/tpm-emulator/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@${PREFIX}/var@/var@g' -e 's@post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d@' /usr/ports/security/trousers/Makefile
sed -i_ -e 's@ var@ /var@' /usr/ports/security/trousers/pkg-plist
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' -e 's@${PREFIX}/etc@/etc@' /usr/ports/print/libpaper/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@${LOCALBASE}/etc@/etc@g' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}/etc/pam.d ${STAGEDIR}/etc/devd@' -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc@' /usr/ports/print/cups/Makefile
sed -i_ -e 's@%%AVAHI%%man@%%AVAHI%%%%MANPREFIX%%/man@' /usr/ports/print/cups/pkg-plist
sed -i_ -e 's@^man/@%%MANPREFIX%%/man/@' /usr/ports/graphics/jbigkit/pkg-plist
sed -i_ -e 's@%%JPEG%%man@%%JPEG%%%%MANPREFIX%%/man@' /usr/ports/graphics/jpeg-turbo/pkg-plist
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@g' /usr/ports/graphics/tiff/Makefile
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@' /usr/ports/x11-fonts/encodings/Makefile
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/x11-toolkits/pango/Makefile
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps/ ${STAGEDIR}/usr/share/applications/@' /usr/ports/devel/qt5-assistant/Makefile /usr/ports/devel/qt5-designer/Makefile
sed -i_ -e 's@usr/local@usr@' /usr/ports/multimedia/gstreamer1/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc\n&@' /usr/ports/audio/alsa-lib/Makefile
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/sysutils/smartmontools/Makefile
sed -i_ -e 's@^post-patch:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-plugin-oauth2/Makefile
sed -i_ -e 's@^\.include@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-kwallet-extension/Makefile
sed -i_ -e 's@DIR="etc@DIR="/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}${PREFIX}/libdata/ldconfig@' -e 's@MANDIR="man"@MANDIR="${MANPREFIX}/man"@' /usr/ports/databases/mysql57-server/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig/\n&@' /usr/ports/graphics/graphviz/Makefile /usr/ports/databases/mysql57-client/Makefile
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm90/Makefile
sed -i_ -e 's@^.*C_INCLUDE_PATH=.*$@\\@' -e 's@^.*CPLUS_INCLUDE_PATH=.*$@\\@' /usr/ports/www/qt5-webengine/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@^post-install:@pre-install:\n\tmkdir -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/multimedia/webcamd/Makefile
sed -i_ -e 's@%%PREFIX%%@@' /usr/ports/multimedia/webcamd/files/webcamd.conf.in
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/share/applications\n&@' /usr/ports/x11/xterm/Makefile
for p in setxkbmap smproxy xcursorgen appres xf86dga iceauth sessreg xauth xbacklight xcmsdb xdpyinfo xdriinfo xev xgamma xhost xinput xkbevd xkill xlsatoms xlsclients xmodmap xprop xrdb xrefresh xset xsetroot xvinfo xwd xwininfo xwud; do sed -i_ -e 's@ man/@ ${MANPREFIX}/man/@' /usr/ports/x11/$p/Makefile; done
sed -i_ -e 's@ man/@ ${MANPREFIX}/man/@' /usr/ports/x11-fonts/bdftopcf/Makefile
sed -i_ -e 's@man/man1@${MANPREFIX}/man/man1@' /usr/ports/x11/xrandr/Makefile /usr/ports/x11/xkbcomp/Makefile /usr/ports/print/xpdfopen/Makefile
sed -i_ -e 's@man/man1@${MANPREFIX}/man/man1@g' /usr/ports/x11/xpr/Makefile
sed -i_ -e 's@^post-patch:@CONFIGURE_ARGS+= --sysconfdir=/etc\n&@' /usr/ports/x11/xinit/Makefile
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@' -e 's@${PREFIX}/etc@/etc@g' -e 's@--without-doxygen@--sysconfdir=/etc &@' /usr/ports/x11-servers/xorg-server/Makefile
sed -i_ -e 's@%%SUID%%man@%%SUID%%%%MANPREFIX%%/man@' /usr/ports/x11-servers/xorg-server/pkg-plist
sed -i_ -e 's@man/man4@${MANPREFIX}/man/man4@g' /usr/ports/x11-drivers/xf86-video-scfb/Makefile
sed -i_ -e 's@^\t\tman/@\t\t${MANPREFIX}/man/@' /usr/ports/x11-fonts/mkfontscale/Makefile
sed -i_ -e 's@%%man/@%%%%MANPREFIX%%/man/@' /usr/ports/print/freetype2/pkg-plist
sed -i_ -e 's@%%MANPAGES%%man@%%MANPAGES%%%%MANPREFIX%%/man@' /usr/ports/devel/glib20/pkg-plist /usr/ports/graphics/gtk-update-icon-cache/pkg-plist /usr/ports/devel/dbus/pkg-plist /usr/ports/sysutils/polkit/pkg-plist /usr/ports/security/p11-kit/pkg-plist
sed -i_ -e '/PLIST_FILES/s@man/man@${MANPREFIX}/man/man@g' /usr/ports/textproc/html2text/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@^post-patch:@UNUSED-post-patch:@' -e 's@MAN1PREFIX@MANPREFIX@g' /usr/ports/misc/qtchooser/Makefile
sed -i_ -e 's@%%EVDEV%%man@%%EVDEV%%%%MANPREFIX%%/man@' -e 's@%%WAYLAND%%man@%%WAYLAND%%%%MANPREFIX%%/man@' -e 's@%%X11%%man@%%X11%%%%MANPREFIX%%/man@' /usr/ports/x11/libxkbcommon/pkg-plist /usr/ports/audio/pulseaudio/pkg-plist
sed -i_ -e 's@%%TCLMAN%%man@%%TCLMAN%%%%MANPREFIX%%/man@' /usr/ports/lang/tcl86/pkg-plist
sed -i_ -e 's@%%TCL%%man@%%TCL%%%%MANPREFIX%%/man@' /usr/ports/databases/sqlite3/pkg-plist
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/databases/sqlite3/Makefile /usr/ports/math/fftw3/Makefile /usr/ports/audio/flac/Makefile
sed -i_ -e 's@DEF%%man@DEF%%%%MANPREFIX%%/man@' /usr/ports/math/fftw3/pkg-plist
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc@' -e 's@${PREFIX}/etc@/etc@g' /usr/ports/sysutils/polkit/Makefile /usr/ports/audio/pulseaudio/Makefile /usr/ports/accessibility/speech-dispatcher/Makefile /usr/ports/x11-toolkits/gtk20/Makefile /usr/ports/graphics/colord/Makefile
sed -i_ -e '/openssl.cnf/d' -e '/|\/var/d' -e 's@GNU_CONFIGURE.*$@&\nCONFIGURE_ARGS= --sysconfdir=/etc@' /usr/ports/shells/bash-completion/Makefile
sed -i_ -e '/${REINPLACE_CMD} -E/,+23d' -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc --mandir=${MANPREFIX}/man@' /usr/ports/security/gnutls/Makefile
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc --mandir=${MANPREFIX}/man@' -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/x11-toolkits/gtk30/Makefile
sed -i_ -e 's@%%BROADWAY%%man@%%BROADWAY%%%%MANPREFIX%%/man@' /usr/ports/x11-toolkits/gtk30/pkg-plist
sed -i_ -e 's@%%WKS_SERVER%%man@%%WKS_SERVER%%%%MANPREFIX%%/man@' /usr/ports/security/gnupg/pkg-plist
sed -i_ -e 's@%%IF_DEFAULT%%man@%%IF_DEFAULT%%%%MANPREFIX%%/man@' /usr/ports/lang/ruby27/pkg-plist
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/multimedia/ffmpeg/Makefile
sed -i_ -e 's@man/man@%%MANPREFIX%%/&@' /usr/ports/x11-servers/xwayland-devel/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e '/^post-patch:/,+2d' /usr/ports/audio/openal-soft/Makefile
sed -i_ -e 's@^post-install:@post-patch:\n\tsed -i_ -e "s,^mandir.*,mandir = ${MANPREFIX}/man," ${WRKSRC}/Makefile\n&@' /usr/ports/databases/lmdb/Makefile
sed -i_ -e 's@^CMAKE_OFF.*@&\nCMAKE_ARGS+= -DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man@' /usr/ports/archivers/libzip/Makefile /usr/ports/graphics/exiv2/Makefile /usr/ports/devel/zziplib/Makefile
sed -i_ -e 's@^%%MANPAGES%%man@%%MANPAGES%%%%MANPREFIX%%/man@' /usr/ports/devel/zziplib/pkg-plist
sed -i_ -e 's@^CMAKE_ARGS=@& -DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man@' -e 's@man/man1@${MANPREFIX}/&@' /usr/ports/graphics/libqrencode/Makefile
sed -i_ -e 's@USES=.*@&\nCMAKE_ARGS+= -DOPENJPEG_INSTALL_MAN_DIR=${MANPREFIX}/man@' /usr/ports/graphics/openjpeg15/Makefile
sed -i_ -e 's@^post-patch:@CONFIGURE_ARGS+= --sysconfdir=/etc --mandir=${MANPREFIX}/man\n&@' /usr/ports/graphics/graphviz/Makefile
sed -i_ -E 's@%%(TCL|GO|GUILE|LUA|SMYRNA|ANN|XPM)%%man@%%\1%%%%MANPREFIX%%/man@' /usr/ports/graphics/graphviz/pkg-plist
sed -i_ -e 's@USES=.*@&\nCMAKE_ARGS+= -DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man@' /usr/ports/editors/editorconfig-core-c/Makefile /usr/ports/graphics/jasper/Makefile
sed -i_ -e '/${LOCALBASE}\/etc/,+1d' /usr/ports/sysutils/signon-qt5/Makefile
sed -i_ -e 's@%%SASLDB%%man@%%SASLDB%%%%MANPREFIX%%/man@' /usr/ports/security/cyrus-sasl2/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@g' /usr/ports/security/cyrus-sasl2/Makefile.common
sed -i_ -e 's@--disable-slapd@& --sysconfdir=/etc --mandir=${MANPREFIX}/man @' /usr/ports/net/openldap24-server/Makefile
sed -i_ -e 's@^man@%%MANPREFIX%%/man@' /usr/ports/net/openldap24-server/pkg-plist.client
sed -i_ -e 's@%%man@%%%%MANPREFIX%%/man@' /usr/ports/sysutils/upower/pkg-plist /usr/ports/graphics/jasper/pkg-plist /usr/ports/graphics/lcms/pkg-plist /usr/ports/net/samba412/pkg-plist
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' /usr/ports/sysutils/upower/Makefile
sed -i_ -e 's@${LOCALBASE}/etc@/etc@g' -e 's@-DUID_MAX@-DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man &@' -e 's@${MKDIR}@& ${STAGEDIR}/etc/rc.d @' /usr/ports/x11/sddm/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/net/openslp/Makefile
sed -i_ -e 's@man/man3@${MANPREFIX}/man/man3@' /usr/ports/devel/talloc/Makefile
sed -i_ -e 's@man/man8@${MANPREFIX}/man/man8@' /usr/ports/databases/tdb/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@${PREFIX}/man@${MANPREFIX}/man@g' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' -e '/FRUIT_PLIST/s@man@${MANPREFIX}/man@' /usr/ports/net/samba412/Makefile
sed -i_ -e 's@USE_LDCONFIG.*@&\npost-patch:\n\tsed -i_ -e "s,/man/man,/share/man/man," ${WRKSRC}/Makefile\n@' /usr/ports/audio/gsm/Makefile
sed -i_ -e 's@^post-build:@CONFIGURE_ARGS+= --sysconfdir=/etc\npre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/audio/jack/Makefile
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/multimedia/libkate/Makefile
sed -i_ -e 's@s|$$|.gz|@&;s|man/man3|${MANPREFIX}/\&|@' /usr/ports/math/lapack/Makefile
sed -i_ -e 's@^WRKSRC=.*@&\npost-patch:\n\tsed -i_ -e "/^mandir/s,/man,/share/man," ${WRKSRC}/Makefile\n@' /usr/ports/multimedia/librtmp/Makefile
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/audio/sndio/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' /usr/ports/graphics/ImageMagick7/Makefile
sed -i_ -e '/PLIST_FILES/s@man/@${MANPREFIX}/man/@' /usr/ports/sysutils/cpdup/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/emulators/open-vm-tools/Makefile
sed -i_ -e 's@${LOCALBASE}/etc@/etc@g' -e 's@${PREFIX}/etc@/etc@g' -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/net/openntpd/Makefile
sed -i_ -e 's@man/@${MANPREFIX}/man/@' /usr/ports/sysutils/pv/Makefile
sed -i_ -e 's@\tman/@\t${MANPREFIX}/man/@' -e 's@${PREFIX}/man@${MANPREFIX}/man@' -e 's@do-install:@&\n\t${MKDIR} -p ${STAGEDIR}/usr/share/applications@' /usr/ports/net/wpa_supplicant_gui/Makefile
sed -i_ -e 's@GNU_CONFIGURE.*@&\nCONFIGURE_ARGS=--sysconfdir=/etc@' /usr/ports/devel/xdg-user-dirs/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@post-install:@&\n\tmkdir -p ${STAGEDIR}/etc/pam.d@' /usr/ports/security/sudo/Makefile
sed -i_ -e '/^CPPFLAGS/d;/^LDFLAGS/d;s@CONFIGURE_ARGS=@& --sysconfdir=/etc @;s@FLAVORS=@FLAVOR=tiny\n&@;s@${PREFIX}/etc@/etc@g;s@^post-install:@&\n\tmkdir -p ${STAGEDIR}/etc/rc.d@' /usr/ports/devel/git/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g;s@^post-install-PAM-on:@&\n\tmkdir -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}/etc/pam.d@' /usr/ports/x11/slim/Makefile
sed -i_ -e '/^+/s@PREFIX}/man@PREFIX}/share/man@' /usr/ports/x11/slim/files/patch-CMakeLists.txt
sed -i_ -e 's@%%PREFIX%%/etc@/etc@g' /usr/ports/x11/slim/files/slim.in
rm -f /usr/ports/x11/slim/files/patch-CMakeLists.txt_ /usr/ports/x11/slim/files/patch-slim.1 /usr/ports/x11/slim/files/slim.in_
sed -i_ -e 's@post-install:@&\n\tmkdir -p ${STAGEDIR}/usr/libdata/ldconfig@' /usr/ports/devel/qscintilla2-qt5/Makefile
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@' /usr/ports/graphics/libmng/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@' /usr/ports/lang/rust/Makefile /usr/ports/sysutils/dmidecode/Makefile
sed -i_ -e 's@${STAGEDIR}${PREFIX}@${STAGEDIR}@g' /usr/ports/x11-fonts/dejavu/Makefile
sed -i_ -e 's@%%FCDIR@/&@' /usr/ports/x11-fonts/dejavu/pkg-plist
sed -i_ -e 's@etc@/etc@;s@${PREFIX}/${CONF@${CONF@g' /usr/ports/x11-fonts/wqy/Makefile
sed -i_ -e 's@${PREFIX}/man@${MANPREFIX}/man@;s@^post-patch:@LIBLZMA_LIBS+= -llz4\n&\n\tsed -i_ -e "s,^etcdir.*,etcdir=/etc," ${WRKSRC}/mk/defs.mk.in@;s@--mandir=@--sysconfdir=/etc &@' -e 's@post-install:@pre-install:\n\tmkdir -p ${STAGEDIR}/etc ${STAGEDIR}/usr/libdata/ldconfig\n&@' /usr/ports/ports-mgmt/pkg/Makefile
sed -i_ -e 's@man/man3@${MANPREFIX}/&@' /usr/ports/devel/libsysinfo/Makefile
sed -i_ '5,13d' /usr/ports/math/qhull/files/patch-CMakeLists.txt
rm -f /usr/ports/math/qhull/files/patch-CMakeLists.txt_
sed -i_ -e '/INSTALL_MAN/s@PREFIX@MANPREFIX@' /usr/ports/archivers/minizip/Makefile
sed -i_ -e '/^PLIST_FILES/,+3s@man/man1@${MANPREFIX}/&@g' /usr/ports/archivers/zip/Makefile
sed -i_ -e '$s@^.*$@CMAKE_ARGS+=\t-DCMAKE_INSTALL_MANDIR=${MANPREFIX}/man\n&@' /usr/ports/math/cgal/Makefile
sed -i_ -e 's@${PREFIX}/etc@/etc@g' /usr/ports/databases/postgresql14-server/Makefile /usr/ports/graphics/gdal/Makefile
sed -i_ -e 's@^man@%%MANPREFIX%%/man@' /usr/ports/databases/postgresql12-server/pkg-plist-client /usr/ports/databases/postgresql12-server/pkg-plist-server
sed -i_ -e 's@ man/man1/@ %%MANPREFIX%%/man/man1/@' /usr/ports/www/node/pkg-plist
sed -i_ -e 's@^post-install:@&\n\tgzip <${STAGEDIR}/usr/man/man1/node.1 >${STAGEDIR}${MANPREFIX}/man/man1/node.1.gz@' /usr/ports/www/node/Makefile

# Port out of date?
sed -i_ -e 's@2391904@2446510@' -e 's@a65b84821765cfd4bb8bf8c05e4279a9d81130da4eb8741ef2690064c57610cf@1eaa672dfa1ac921c795117b29b830eb84902a66248ef08d461f093305e2aaf5@' /usr/ports/math/lapack/distinfo
echo >/usr/ports/math/lapack/files/manpages
