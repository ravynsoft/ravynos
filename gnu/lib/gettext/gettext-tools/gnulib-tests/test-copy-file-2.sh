#!/bin/sh

# Test copy-file on the file system of the build directory, which may be
# a local file system or NFS mounted.

. "${srcdir=.}/init.sh"; path_prepend_ .

TMPDIR=`pwd`
export TMPDIR

$BOURNE_SHELL "${srcdir}/test-copy-file.sh"
ret1=$?
NO_STDERR_OUTPUT=1 $BOURNE_SHELL "${srcdir}/test-copy-file.sh"
ret2=$?
case $ret1 in
  77 ) Exit $ret2 ;;
  * ) Exit $ret1 ;;
esac
