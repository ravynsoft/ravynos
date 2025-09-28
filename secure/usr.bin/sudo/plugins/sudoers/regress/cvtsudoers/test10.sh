#!/bin/sh
#
# Test command defaults filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d command $TESTDIR/sudoers

exit 0
