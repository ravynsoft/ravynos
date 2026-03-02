#!/bin/sh
#
# Test sudoers owner check
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1
$TESTSUDOERS -U 1 root id <<EOF
@include $TESTDIR/test2.inc
EOF

exit 0
