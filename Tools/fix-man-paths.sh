#!/bin/sh
# Fix various ports to build correctly as Airyx base OS

find /usr/ports -name pkg-plist_ -o -name Makefile_ -exec mv -vf {} $(basename {} _) \;

# Uses/gnome
sed -i_ -e 's@${LOCALBASE}/etc/gnome.subr@/etc/gnome.subr@' /usr/ports/Mk/Uses/gnome.mk

# Prevent reordering /usr/include to the end of system includes - it breaks C++
sed -i_ -e 's/^CPPFLAGS/# CPPFLAGS/' -e 's/^CFLAGS/# CFLAGS/' -e 's/^CXXFLAGS/# CXXFLAGS/' /usr/ports/Mk/Uses/localbase.mk

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

# at-spi2-core
sed -i_ -e 's@^etc/@/etc/@' /usr/ports/accessibility/at-spi2-core/pkg-plist

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
