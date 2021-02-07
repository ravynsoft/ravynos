#! /bin/sh
#
#   Test for Objective-C precompiled headers
#
#   Copyright (C) 2007 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

# Check if GCC supports precompiled headers for Objective-C or not.

# You should execute this shell scripts after setting the following
# environment variables:
#
#  CC, CFLAGS, CPPFLAGS, LDFLAGS, LIBS
#
# ./configure at the top-level will set them for us; you need to
# set them manually if you want to run the test manually.

# The script will execute and:
#   return 0 if gcc supports ObjC precompiled headers
#   return 1 if gcc does not

# The script takes a single argument, which is the directory where
# the temporary files and the log file will be written.  If there
# is no argument specified, ./ will be used.

# This is the file where everything will be logged
gs_builddir="$1"

if test "$gs_builddir" = ""; then
  gs_builddir="."
fi

gs_logfile="$gs_builddir/config-precomp-test.log"

# Clear logs
rm -f "$gs_logfile"

# Clear compilation results
rm -f "$gs_builddir/config-precomp-test.h.gch" "$gs_builddir/config-precomp-test.out"

echo "** Environment" >>"$gs_logfile" 2>&1
echo " CC: $CC" >>"$gs_logfile" 2>&1
echo " CFLAGS: $CFLAGS" >>"$gs_logfile" 2>&1
echo " CPPFLAGS: $CPPFLAGS" >>"$gs_logfile" 2>&1
echo " LDFLAGS: $LDFLAGS" >>"$gs_logfile" 2>&1
echo " LIBS: $LIBS" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1
echo " current directory: `pwd`" >>"$gs_logfile" 2>&1
echo " log file: $gs_logfile" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1

# Get rid of '-x objective-c' in CFLAGS that we don't need and would
# prevent our '-x objective-c-headers' flag from working.
CFLAGS=`echo $CFLAGS | sed -e 's/-x objective-c//'`
echo " CFLAGS without -x objective-c: $CFLAGS" >>"$gs_logfile" 2>&1

echo "" >>"$gs_logfile" 2>&1

if test "$CC" = ""; then
  echo "CC is not set: failure" >>"$gs_logfile" 2>&1
  exit 1
fi

# Try to compile the file first.
echo "** Compile the file without precompiled headers" >>"$gs_logfile" 2>&1
echo "$CC -o \"$gs_builddir/config-precomp-test.out\" $CFLAGS $CPPFLAGS $LDFLAGS config-precomp-test.m $LIBS" >>"$gs_logfile" 2>&1
$CC -o "$gs_builddir/config-precomp-test.out" $CFLAGS $CPPFLAGS $LDFLAGS config-precomp-test.m $LIBS >>"$gs_logfile" 2>&1
if test ! "$?" = "0"; then
  echo "Failure" >>"$gs_logfile" 2>&1
  rm -f "$gs_builddir/config-precomp-test.out"
  exit 1
fi
echo "Success" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1

# Now try to preprocess the header
echo "** Preprocess the header" >>"$gs_logfile" 2>&1
echo "$CC -o \"$gs_builddir/config-precomp-test.h.gch\" -c -x objective-c-header $CFLAGS $CPPFLAGS $LDFLAGS config-precomp-test.h" >>"$gs_logfile" 2>&1
$CC -o "$gs_builddir/config-precomp-test.h.gch" -c -x objective-c-header $CFLAGS $CPPFLAGS $LDFLAGS config-precomp-test.h >>"$gs_logfile" 2>&1
if test ! "$?" = "0"; then
  echo "Failure" >>"$gs_logfile" 2>&1
  rm -f "$gs_builddir/config-precomp-test.out" "$gs_builddir/config-precomp-test.h.gch"
  exit 1
fi
echo "Success" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1

# Now try to compile again with the preprocessed header.  It might get ignored - which is fine.
echo "** Compile the file with precompiled headers" >>"$gs_logfile" 2>&1
echo "$CC -o \"$gs_builddir/config-precomp-test.out\" $CFLAGS $CPPFLAGS $LDFLAGS -I\"$gs_builddir\" config-precomp-test.m $LIBS" >>"$gs_logfile" 2>&1
$CC -o "$gs_builddir/config-precomp-test.out" $CFLAGS $CPPFLAGS $LDFLAGS -I"$gs_builddir" config-precomp-test.m $LIBS >>"$gs_logfile" 2>&1
if test ! "$?" = "0"; then
  echo "Failure" >>"$gs_logfile" 2>&1
  rm -f "$gs_builddir/config-precomp-test.out" "$gs_builddir/config-precomp-test.h.gch"
  exit 1
fi
echo "Success" >>"$gs_logfile" 2>&1

# Everything looks OK.
exit 0
