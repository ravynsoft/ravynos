#!/bin/sh
#
# Test user and host filters
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -m user=millert,host=hercules $TESTDIR/sudoers

exit 0
