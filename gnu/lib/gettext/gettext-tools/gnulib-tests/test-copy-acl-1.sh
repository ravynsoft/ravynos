#!/bin/sh

# Test copy-acl on the file system of /var/tmp, which usually is a local
# file system.

. "${srcdir=.}/init.sh"; path_prepend_ .

if test -d /var/tmp; then
  TMPDIR=/var/tmp
else
  TMPDIR=/tmp
fi
test -d $TMPDIR || Exit 77
export TMPDIR

$BOURNE_SHELL "${srcdir}/test-copy-acl.sh"

Exit $?
