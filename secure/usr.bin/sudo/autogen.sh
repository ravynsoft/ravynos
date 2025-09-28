#!/bin/sh

# OpenBSD may have multiple versions of autoconf and automake installed
# If the user hasn't chosen one themselves, we do here.
if [ "`/usr/bin/uname 2>&1`" = "OpenBSD" ]; then
    if [ X"$AUTOMAKE_VERSION" = X"" ]; then
        AUTOMAKE_VERSION=1.16; export AUTOMAKE_VERSION
    fi
    if [ X"$AUTOCONF_VERSION" = X"" ]; then
        AUTOCONF_VERSION=2.71; export AUTOCONF_VERSION
    fi
fi

set -ex

autoreconf -f -i -v -Wall -I m4

rm -rf autom4te.cache

exit 0
