#!/bin/sh
# Fix various ports to build correctly as Airyx base OS

find /usr/ports -name pkg-plist_ -o -name Makefile_ -exec mv -vf {} $(basename {} _) \;

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

# Qt5 Core
sed -i_ -e 's@^post-install:@pre-install:\n\t${MKDIR} -p ${STAGEDIR}${PREFIX}/libdata/ldconfig\n&@' /usr/ports/devel/qt5-core/Makefile

# Fontconfig
sed -i_ -e 's@^FCDEFAULTFONTS.*@FCDEFAULTFONTS= /System/Library/Fonts /Library/Fonts@' -e 's@${STAGEDIR}${PREFIX}@${STAGEDIR}@g' /usr/ports/x11-fonts/fontconfig/Makefile
sed -i -e 's@^post-install:@&\n\tMESON_INSTALL_DESTDIR_PREFIX="" /usr/bin/python3 ${WRKSRC}/conf.d/link_confs.py /etc/fonts/conf.avail ${STAGEDIR}/etc/fonts/conf.d 10-hinting-slight.conf 10-scale-bitmap-fonts.conf 20-unhint-small-vera.conf 30-metric-aliases.conf 40-nonlatin.conf 45-generic.conf 45-latin.conf 49-sansserif.conf 50-user.conf 51-local.conf 60-generic.conf 60-latin.conf 65-fonts-persian.conf 65-nonlatin.conf 69-unifont.conf 80-delicious.conf 90-synthetic.conf@' /usr/ports/x11-fonts/fontconfig/Makefile
sed -i_ -e 's@^etc/@/etc/@' -e 's@%etc@%/etc@g' -e 's@ etc@ /etc@g' /usr/ports/x11-fonts/fontconfig/pkg-plist
cat >/usr/ports/x11-fonts/fontconfig/files/patch-conf.d_meson.build_2 <<EOT
--- conf.d/meson.build.orig     2021-06-15 21:56:10.019092000 -0400
+++ conf.d/meson.build  2021-06-15 22:01:50.208793000 -0400
@@ -60,12 +60,6 @@
 ]

 install_data(conf_files, install_dir: join_paths(get_option('sysconfdir'), 'fonts/conf.avail'))
-
-meson.add_install_script('link_confs.py',
-  join_paths(get_option('prefix'), get_option('sysconfdir'), 'fonts/conf.avail'),
-  join_paths(get_option('sysconfdir'), 'fonts', 'conf.d'),
-  conf_links,
-)

 # 35-lang-normalize.conf
 orths = []
EOT

