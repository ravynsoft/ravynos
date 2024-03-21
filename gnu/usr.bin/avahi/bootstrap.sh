#!/bin/sh

# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

FLAGS="--sysconfdir=/etc --localstatedir=/var --enable-tests --enable-compat-howl --enable-compat-libdns_sd"

# Feel free to add your own custom flags in here -Lathiat

case `uname -s` in
    Darwin)
    export LIBTOOLIZE=/opt/local/bin/glibtoolize
    export CFLAGS="-I/opt/local/include"
    export LDFLAGS="-L/opt/local/lib"
    export PKG_CONFIG_PATH="/opt/local/lib/pkgconfig"
    FLAGS="$FLAGS --prefix=/opt/local --disable-pygtk"
    ;;
    FreeBSD)
    cp /usr/local/share/aclocal/libtool15.m4 common
    cp /usr/local/share/aclocal/pkg.m4 common
    export LIBTOOLIZE=/usr/local/bin/libtoolize15
    export CFLAGS="-I/usr/local/include"
    export LDFLAGS="-L/usr/local/lib"
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"
    FLAGS="$FLAGS --prefix=/opt/ --with-distro=none --disable-python --disable-dbus --disable-glib --disable-gtk --disable-libevent"
    ;;
    NetBSD)
    export LIBTOOLIZE=libtoolize
    export CFLAGS="-I/usr/pkg/include"
    export LDFLAGS="-L/usr/pkg/lib"
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"
    FLAGS="$FLAGS --disable-monodoc --disable-mono --disable-qt4 --disable-xmltoman --prefix=/opt --with-distro=none --disable-python --disable-glib --disable-gtk --disable-manpages --disable-libevent"
    ;;
    Linux)
    ;;
esac

case "$USER" in
    lathiat|trentl)
    FLAGS="$FLAGS"
    ;;
    sebest)
    FLAGS="$FLAGS --disable-monodoc --enable-dbus=no --enable-mono=no --enable-qt4=no --sysconfdir=/etc --localstatedir=/var --prefix=/usr  --disable-manpages --disable-xmltoman"
    ;;
esac

CFLAGS="$CFLAGS -g -O0" exec ./autogen.sh $FLAGS "$@"
