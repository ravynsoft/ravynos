#!/bin/sh
#
# Test cvtsudoers merge:
#  * two files, each bound to a host
#  * only difference is a conflicting secure_path definition
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -f sudoers -l /dev/null xerxes:${TESTDIR}/sudoers3 xyzzy:${TESTDIR}/sudoers4
