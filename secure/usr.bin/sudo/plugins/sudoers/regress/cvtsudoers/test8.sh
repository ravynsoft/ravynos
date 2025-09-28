#!/bin/sh
#
# Test runas defaults filtering
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -s aliases,privileges -d runas $TESTDIR/sudoers

exit 0
