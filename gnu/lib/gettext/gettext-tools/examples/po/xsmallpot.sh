#!/bin/sh
#
# Copyright (C) 2003-2005, 2012-2014, 2018-2019 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Usage: xsmallpot.sh srcdir hello-foo [hello-foobar.pot]

set -e

# Nuisances.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

test $# = 2 || test $# = 3 || { echo "Usage: xsmallpot.sh srcdir hello-foo [hello-foobar.pot]" 1>&2; exit 1; }
srcdir=$1
directory=$2
potfile=${3-$directory.pot}

abs_srcdir=`cd "$srcdir" && pwd`

cd ..
rm -rf tmp-$directory
cp -p -r "$abs_srcdir"/../$directory tmp-$directory
chmod -R u+w tmp-$directory
cd tmp-$directory
case $directory in
  hello-c++-kde)
    cat > configure.ac <<EOF
AC_INIT
AC_CONFIG_AUX_DIR(admin)
AM_INIT_AUTOMAKE([$directory], 0)
AC_PROG_CXX
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([m4/Makefile])
AC_OUTPUT
EOF
    autopoint -f
    aclocal -I m4
    autoconf
    automake -a -c
    ./configure
    ;;
  hello-objc-gnustep)
    ./autogen.sh
    ;;
  hello-pascal)
    ./autogen.sh
    ./configure
    # Build hello.rsj.
    make hello.rsj
    ;;
  *)
    grep '^\(AC_INIT\|AC_CONFIG\|AC_PROG_\|AC_SUBST(.*OBJC\|AM_INIT\|AM_CONDITIONAL\|AM_GNU_GETTEXT\|AM_PO_SUBDIRS\|AC_OUTPUT\)' configure.ac > tmp-configure.ac
    mv -f tmp-configure.ac configure.ac
    ./autogen.sh
    ./configure
    ;;
esac
cd po
make $potfile
sed -e "/^#:/ {
s, \\([^ ]\\), $directory/\\1,g
}" < $potfile > "$abs_srcdir"/$potfile
cd ..
cd ..
rm -rf tmp-$directory
