#!/bin/sh
#
# Test cvtsudoers merge
#  * three files, each bound to a host
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -f sudoers -l /dev/null xerxes:${TESTDIR}/sudoers1 xyzzy:${TESTDIR}/sudoers2 plugh:${TESTDIR}/sudoers2
