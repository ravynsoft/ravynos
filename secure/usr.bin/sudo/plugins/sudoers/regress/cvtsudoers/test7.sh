#!/bin/sh
#
# Test user defaults filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d user $TESTDIR/sudoers

exit 0
