#!/bin/sh
#
# Test host defaults filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d host $TESTDIR/sudoers

exit 0
