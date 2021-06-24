#!/bin/sh
# Fix various ports to build correctly as Airyx base OS
# This script is a Hideous Hack to get things building quickly. It should be
# replaced (as much as possible) with tuning build options and other
# supported techniques that help ensure consistent settings across ports

# Revert any previous changes
find /usr/ports -name pkg-plist_ -o -name Makefile_ -exec mv -vf {} $(basename {} _) \;

# Make sure everything uses Airyx paths
sed -i_ -e 's@${LOCALBASE}/etc/gnome.subr@/etc/gnome.subr@' /usr/ports/Mk/Uses/gnome.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@' /usr/ports/Mk/Uses/display.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/Mk/Uses/xorg-cat.mk
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@g' -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/Mk/Uses/fonts.mk
sed -i_ -e 's@${PREFIX}/etc/@/etc/@' /usr/ports/Mk/bsd.port.mk

# Prevent reordering /usr/include to the end of system includes - it breaks C++
sed -i_ -e 's/^CPPFLAGS/# CPPFLAGS/' -e 's/^CFLAGS/# CFLAGS/' -e 's/^CXXFLAGS/# CXXFLAGS/' /usr/ports/Mk/Uses/localbase.mk
sed -i_ -e 's@${QT_LIBDIR} \\@${QT_LIBDIR}@' -e 's@^.*--with-extra-includes.*$@@' -e 's@^.*--with-extra-libs.*$@@' /usr/ports/Mk/Uses/qmake.mk
sed -i_ -e 's@^CONFIGURE_ARGS@# &@' /usr/ports/Mk/Uses/xorg.mk

# Perl 5.32
sed -i_ -e 's/${PREFIX}\/etc/\/etc/' /usr/ports/lang/perl5.32/Makefile
sed -i_ -e 's/^\t*${INSTALL_DATA} ${WRKDIR}.*$/\t${MKDIR} -p ${STAGEDIR}\/etc\/man.d ${STAGEDIR}\/usr\/libdata\/ldconfig\n&/' /usr/ports/lang/perl5.32/Makefile
sed -i_ -e 's/^etc/\/etc/' /usr/ports/lang/perl5.32/pkg-plist

# expat2
sed -i_ -e 's/^\t${INSTALL_MAN}/\t${MKDIR} ${STAGEDIR}\/usr\/man\/man1\n\tsed -i_ -e "s@install_sh =.*@install_sh = \/bin\/sh ${WRKSRC}\/conftools\/install-sh@\" ${WRKSRC}/Makefile\n&/' /usr/ports/textproc/expat2/Makefile

# xmlcatmgr
sed -i_ -e 's@^\.include <bsd\.port\.mk>@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/xml ${STAGEDIR}${PREFIX}/share/sgml\n&@' /usr/ports/textproc/xmlcatmgr/Makefile

# qt5-core
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/qt5-core/Makefile

# Fontconfig
sed -i_ -e 's@^FCDEFAULTFONTS.*@FCDEFAULTFONTS= /System/Library/Fonts /Library/Fonts@' -e 's@${STAGEDIR}${PREFIX}@${STAGEDIR}@g' /usr/ports/x11-fonts/fontconfig/Makefile
sed -i_ -e 's@^etc/@/etc/@' -e 's@%etc@%/etc@g' -e 's@ etc@ /etc@g' /usr/ports/x11-fonts/fontconfig/pkg-plist

# LLVM10
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm10/Makefile

# html2text
sed -i_ -e 's@^do-install:@&\n\t${MKDIR} -p ${STAGEDIR}${MANPREFIX}/man/man1 ${STAGEDIR}${MANPREFIX}/man/man5@' /usr/ports/textproc/html2text/Makefile

# dbus
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/devel/dbus/Makefile

# at-spi2-core, plasma5-plasma-browser-integration, plasma5-polkit-kde-agent-1
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/accessibility/at-spi2-core/pkg-plist /usr/ports/www/plasma5-plasma-browser-integration/pkg-plist /usr/ports/sysutils/plasma5-polkit-kde-agent-1/pkg-plist /usr/ports/sysutils/plasma5-powerdevil/pkg-plist

# qt5-widgets
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps\n&@' /usr/ports/x11-toolkits/qt5-widgets/Makefile

# sqlite3
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/man/man1\n&@' /usr/ports/databases/sqlite3/Makefile

# qt5-linguisttools
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/usr/man/man1\n&@' /usr/ports/devel/qt5-linguisttools/Makefile

# kf5-kservice
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/devel/kf5-kservice/pkg-plist

# rust
sed -i_ -e 's@share/man@man@' -e 's@\\1.gz@\\1@' /usr/ports/lang/rust/Makefile

# desktop-file-utils
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/share/applications\n&@' /usr/ports/devel/desktop-file-utils/Makefile

# aspell
sed -i_ -e 's@^etc@/etc@' /usr/ports/textproc/aspell/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/textproc/aspell/Makefile

# fftw3
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/man/man1@' /usr/ports/math/fftw3/Makefile

# gnome_subr
sed -i_ -e 's@etc/@/etc/@' -e 's@${PREFIX}/etc@/etc@' -e 's@^do-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' /usr/ports/sysutils/gnome_subr/Makefile

# p11-kit
sed -i_ -e 's@${PREFIX}/etc@/etc@' /usr/ports/security/p11-kit/Makefile

# tpm-emulator
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/emulators/tpm-emulator/Makefile

# trousers
sed -i_ -e 's@${PREFIX}/etc@/etc@' -e 's@post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d@' /usr/ports/security/trousers/Makefile
sed -i_ -e 's@ etc@ /etc@' -e 's@^man/@share/man/@' -e 's@\.gz$@@' /usr/ports/security/trousers/pkg-plist

# libpaper
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc@' -e 's@${PREFIX}/etc@/etc@' /usr/ports/print/libpaper/Makefile
sed -i_ -e 's@^etc@/etc@' /usr/ports/print/libpaper/pkg-plist

# cups
sed -i_ -e 's@^etc/@/etc/@' -e 's@ etc/@ /etc/@' /usr/ports/print/cups/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@${LOCALBASE}/etc@/etc@g' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}/etc/pam.d ${STAGEDIR}/etc/devd@' /usr/ports/print/cups/Makefile

# kf5-kxmlgui
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/x11-toolkits/kf5-kxmlgui/pkg-plist

# jbigkit
sed -i_ -e 's@^man/@share/man/@' -e 's@\.gz$@@' /usr/ports/graphics/jbigkit/pkg-plist
sed -i_ -e 's@${MANPREFIX}/man@${MANPREFIX}/share/man@g' /usr/ports/graphics/jbigkit/Makefile

# font-bh-ttf
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/x11-fonts/font-bh-ttf/pkg-plist

# font-misc-ethiopic
sed -i_ -e 's@^share/fonts@/System/Library/Fonts@' -e 's@ share/fonts@ /System/Library/Fonts@' /usr/ports/x11-fonts/font-misc-ethiopic/pkg-plist

# font-util
sed -i_ -e 's@^share/fonts@/System/Library/Fonts@' /usr/ports/x11-fonts/font-util/pkg-plist

# encodings
sed -i_ -e 's@^share/fonts@/System/Library/Fonts@' /usr/ports/x11-fonts/encodings/pkg-plist
sed -i_ -e 's@${PREFIX}/share/fonts@/System/Library/Fonts@' /usr/ports/x11-fonts/encodings/Makefile

# pango
sed -i_ -e 's@${LOCALBASE}/share/fonts@/System/Library/Fonts@g' /usr/ports/x11-toolkits/pango/Makefile
sed -i_ -e 's@\.gz$@@' /usr/ports/x11-toolkits/pango/pkg-plist

# kf5-kio, plasma5-discover
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/devel/kf5-kio/pkg-plist /usr/ports/sysutils/plasma5-discover/pkg-plist

# qt5-assistant
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps/ ${STAGEDIR}/usr/share/applications/@' /usr/ports/devel/qt5-assistant/Makefile

# qt5-designer
sed -i_ -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/usr/share/pixmaps/ ${STAGEDIR}/usr/share/applications/@' /usr/ports/devel/qt5-designer/Makefile

# gstreamer1
sed -i_ -e 's@usr/local@usr@' /usr/ports/multimedia/gstreamer1/pkg-plist

# libvdpau, alsa-lib
sed -i_ -e 's@ etc/@ /etc/@' /usr/ports/multimedia/libvdpau/pkg-plist /usr/ports/audio/alsa-lib/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@g' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc\n&@' /usr/ports/audio/alsa-lib/Makefile

# kf5-kdelibs4support, kf5-html, kf5-baloo
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/x11/kf5-kdelibs4support/pkg-plist /usr/ports/www/kf5-khtml/pkg-plist /usr/ports/sysutils/kf5-baloo/pkg-plist

# psutils
sed -i_ -e 's@MANDIR ?=@MANDIR =@' -e 's@$(MANPREFIX)/man@$(MANPREFIX)/share/man@' /usr/ports/print/psutils/files/patch-Makefile.unix
rm -f /usr/ports/print/psutils/files/patch-Makefile.unix_
sed -i_ -e 's@^man/@share/man/@' -e 's@\.gz$@@' /usr/ports/print/psutils/pkg-plist

# graphviz, mysql57-client
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig/\n&@' /usr/ports/graphics/graphviz/Makefile /usr/ports/databases/mysql57-client/Makefile

# plasma5-plasma-workspace, plasma5-kinfocenter, akonadi, plasma5-ksysguard, plasma5-plasma-desktop, smartmontools
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/x11/plasma5-plasma-workspace/pkg-plist /usr/ports/sysutils/plasma5-kinfocenter/pkg-plist /usr/ports/databases/akonadi/pkg-plist /usr/ports/sysutils/plasma5-ksysguard /usr/ports/security/plasma5-kwallet-pam/pkg-plist /usr/ports/x11/plasma5-plasma-desktop/pkg-plist /usr/ports/sysutils/smartmontools/pkg-plist
sed -i_ -e 's@${PREFIX}/etc@/etc@' /usr/ports/sysutils/plasma5-kinfocenter/Makefile /usr/ports/sysutils/smartmontools/Makefile
sed -i_ -e 's@CONFIGURE_ARGS=@& --sysconfdir=/etc @' -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/sysutils/smartmontools/Makefile
sed -i__ -e 's@ etc/@ /etc/@' /usr/ports/sysutils/smartmontools/pkg-plist

# signon-plugin-oauth2
sed -i_ -e 's@^post-patch:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-plugin-oauth2/Makefile

# signon-kwallet-extension
sed -i_ -e 's@^\.include@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/sysutils/signon-kwallet-extension/Makefile

# mysql57-server
sed -i_ -e 's@DIR="etc@DIR="/etc@' -e 's@^post-install:@&\n\t${MKDIR} -p ${STAGEDIR}/etc/rc.d ${STAGEDIR}${PREFIX}/libdata/ldconfig@' /usr/ports/databases/mysql57-server/Makefile

# llvm90
sed -i_ -e 's@post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/llvm90/Makefile

# qt5-webengine
sed -i_ -e 's@^.*C_INCLUDE_PATH=.*$@\\@' -e 's@^.*CPLUS_INCLUDE_PATH=.*$@\\@' /usr/ports/www/qt5-webengine/Makefile

# webcamd
sed -i_ -e 's@^post-install:@pre-install:\n\tmv -f ${STAGEDIR}${PREFIX}/share/man ${STAGEDIR}${PREFIX}\n\tmkdir -p ${STAGEDIR}/etc/rc.d\n&@' /usr/ports/multimedia/webcamd/Makefile

