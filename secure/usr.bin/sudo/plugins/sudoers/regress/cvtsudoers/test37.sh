#!/bin/sh
#
# Test cvtsudoers merge:
#  * two files, each bound to a host
#  * only difference is a conflicting WEBSERVERS definition
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -f sudoers -l /dev/null xerxes:${TESTDIR}/sudoers1 xyzzy:${TESTDIR}/sudoers2
