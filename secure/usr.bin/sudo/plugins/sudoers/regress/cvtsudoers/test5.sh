#!/bin/sh
#
# Test defaults type filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d all $TESTDIR/sudoers

exit 0
