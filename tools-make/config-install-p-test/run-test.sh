#! /bin/sh
#
#   Test for '-p' flag in 'install' program
#
#   Copyright (C) 2009 Free Software Foundation, Inc.
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

# Check if 'install' program supports the '-p' flag.

# You should execute this shell scripts after setting the following
# environment variables:
#
#  INSTALL
#
# ./configure at the top-level will set them for us; you need to
# set them manually if you want to run the test manually.

# We simply try executing
#
#  ${INSTALL} -p test-file test-file2
#
# and then check that test-file2 exists

# The script will execute and:
#   return 0 if install supports '-p'
#   return 1 if install does not support '-p'

# The script takes a single argument, which is the directory where
# the temporary files and the log file will be written.  If there
# is no argument specified, ./ will be used.

# This is the file where everything will be logged
gs_builddir="$1"

if test "$gs_builddir" = ""; then
  gs_builddir="."
fi

gs_logfile="$gs_builddir/config-install-p-test.log"

# Clear logs
rm -f "$gs_logfile"

# Clear test results
rm -f "$gs_builddir/config-install-p-test-file2"

echo "** Environment" >>"$gs_logfile" 2>&1
echo " INSTALL: $INSTALL" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1
echo " current directory: `pwd`" >>"$gs_logfile" 2>&1
echo " log file: $gs_logfile" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1

echo "" >>"$gs_logfile" 2>&1

if test "$INSTALL" = ""; then
  echo "INSTALL is not set: failure" >>"$gs_logfile" 2>&1
  exit 1
fi

# Try to install config-install-p-test-file using '-p'.
echo "** Run $INSTALL -p" >>"$gs_logfile" 2>&1
echo "$INSTALL -p config-install-p-test-file \"$gs_builddir/config-install-p-test-file2\"" >>"$gs_logfile" 2>&1
$INSTALL -p config-install-p-test-file "$gs_builddir/config-install-p-test-file2" >>"$gs_logfile" 2>&1
if test ! "$?" = "0"; then
  echo "Failure" >>"$gs_logfile" 2>&1
  rm -f "$gs_builddir/config-install-p-test-file2"
  exit 1
fi
echo "No error reported by $INSTALL" >>"$gs_logfile" 2>&1
echo "" >>"$gs_logfile" 2>&1

# Now check that the copied file is identical to the original one.
echo "** Checking that the installed file exists" >>"$gs_logfile" 2>&1
if test ! -f "$gs_builddir/config-install-p-test-file2"; then
  echo "Failure - file \"$gs_builddir/config-install-p-test-file2\" missing" >>"$gs_logfile" 2>&1
  exit 1
fi
echo "Success" >>"$gs_logfile" 2>&1

# Everything looks OK.
exit 0
