#!/bin/sh

# Test copy-file on the file system of /var/tmp, which usually is a local
# file system.

. "${srcdir=.}/init.sh"; path_prepend_ .

if test -d /var/tmp; then
  TMPDIR=/var/tmp
else
  TMPDIR=/tmp
fi
test -d $TMPDIR || Exit 77
export TMPDIR

$BOURNE_SHELL "${srcdir}/test-copy-file.sh"
ret1=$?
NO_STDERR_OUTPUT=1 $BOURNE_SHELL "${srcdir}/test-copy-file.sh"
ret2=$?
case $ret1 in
  77 ) Exit $ret2 ;;
  * ) Exit $ret1 ;;
esac
