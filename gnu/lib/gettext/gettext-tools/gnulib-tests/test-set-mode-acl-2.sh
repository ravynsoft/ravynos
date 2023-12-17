#!/bin/sh

# Test set-mode-acl on the file system of the build directory, which may be
# a local file system or NFS mounted.

. "${srcdir=.}/init.sh"; path_prepend_ .

TMPDIR=`pwd`
export TMPDIR

$BOURNE_SHELL "${srcdir}/test-set-mode-acl.sh"

Exit $?
