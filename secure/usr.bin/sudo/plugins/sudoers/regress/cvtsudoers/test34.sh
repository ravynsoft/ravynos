#!/bin/sh
#
# Test cvtsudoers merge
#  * three files, two bound to a host, one global
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -f sudoers -l /dev/null xerxes:${TESTDIR}/sudoers1 ${TESTDIR}/sudoers2 xyzzy:${TESTDIR}/sudoers3
