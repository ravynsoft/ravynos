#!/bin/sh
#
# Test group and host filters
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -m group=wheel,host=blackhole $TESTDIR/sudoers

exit 0
