#!/bin/sh
#
# Test user and host filters, expanding aliases
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -e -m user=millert,host=hercules $TESTDIR/sudoers

exit 0
