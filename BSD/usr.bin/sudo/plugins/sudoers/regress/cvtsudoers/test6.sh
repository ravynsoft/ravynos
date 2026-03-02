#!/bin/sh
#
# Test global defaults filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d global $TESTDIR/sudoers

exit 0
