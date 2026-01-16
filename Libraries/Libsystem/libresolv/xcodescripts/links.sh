#!/bin/bash
set -e -x

if [ "$ACTION" = installhdrs ]; then exit 0; fi

LIBDIR="$DSTROOT"/usr/lib

# installapi runs the script *before* calling tapi... sigh...
mkdir -p "$LIBDIR"

ln -s libresolv.9.tbd "$LIBDIR"/libresolv.tbd
chown -h "$INSTALL_OWNER:$INSTALL_GROUP" "$LIBDIR"/libresolv.tbd

# don't install man pages for installhdrs, installapi, or iOS builds
if [ "$ACTION" = installapi ]; then exit 0; fi

ln -s libresolv.9.dylib "$LIBDIR"/libresolv.dylib
chown -h "$INSTALL_OWNER:$INSTALL_GROUP" "$LIBDIR"/libresolv.dylib
chmod -h 0755 "$LIBDIR"/libresolv.dylib
