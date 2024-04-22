#!/bin/sh

# Build distribution zipfiles for fontconfig on Win32. (This script
# obviously needs to be run in Cygwin or similar.) Separate runtime
# and developer zipfiles.

ZIP=/tmp/fontconfig-@VERSION@.zip
DEVZIP=/tmp/fontconfig-dev-@VERSION@.zip

cd @prefix@
rm -f $ZIP
zip $ZIP -@ <<EOF
bin/libfontconfig-@LIBT_CURRENT_MINUS_AGE@.dll
etc/fonts/fonts.conf
EOF

rm -f $DEVZIP
zip -r -D $DEVZIP -@ <<EOF
etc/fonts/fonts.dtd
include/fontconfig
lib/libfontconfig.dll.a
lib/fontconfig.lib
lib/fontconfig.def
lib/pkgconfig/fontconfig.pc
bin/fc-list.exe
bin/fc-cache.exe
share/man
share/doc/fontconfig
EOF
