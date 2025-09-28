#!/bin/bash

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

AM_VERSION=1.11
AC_VERSION=2.63

run_versioned() {
    local P
    local V

    V=$(echo "$2" | sed -e 's,\.,,g')

    if [ -e "`which $1$V 2> /dev/null`" ] ; then
        P="$1$V"
    else
        if [ -e "`which $1-$2 2> /dev/null`" ] ; then
            P="$1-$2"
        else
            P="$1"
        fi
    fi

    shift 2
    "$P" "$@"
}

set -e

test -n "$srcdir" || srcdir=$(dirname "$0")
test -n "$srcdir" || srcdir=.

olddir=$(pwd)

cd $srcdir

if [ -f .git/hooks/pre-commit.sample -a ! -f .git/hooks/pre-commit ] ; then
    cp -p .git/hooks/pre-commit.sample .git/hooks/pre-commit && \
    chmod +x .git/hooks/pre-commit && \
    echo "Activated pre-commit hook."
fi

if [ "x$1" = "xam" ] ; then
    run_versioned automake "$AM_VERSION" -a -c --foreign
    ./config.status
else 
    rm -rf autom4te.cache
    rm -f config.cache

    rm -f Makefile.am~ configure.ac~
    # Evil, evil, evil, evil hack
    sed 's/read dummy/\#/' `which gettextize` | sh -s -- --copy --force
    test -f Makefile.am~ && mv Makefile.am~ Makefile.am
    test -f configure.ac~ && mv configure.ac~ configure.ac

    test "x$LIBTOOLIZE" = "x" && LIBTOOLIZE=libtoolize

    "$LIBTOOLIZE" -c --force
    run_versioned aclocal "$AM_VERSION" -I common
    run_versioned autoconf "$AC_VERSION" -Wall
    run_versioned autoheader "$AC_VERSION"
    run_versioned automake "$AM_VERSION" -a -c --foreign

    cd "$olddir"
    if test "x$NOCONFIGURE" = "x"; then
        $srcdir/configure "$@"
        make clean
    fi
fi
