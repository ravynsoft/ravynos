#!/bin/sh
#
# Test #include facility
#

: ${TESTSUDOERS=testsudoers}

MYUID=`\ls -ln $TESTDIR/test2.inc | awk '{print $3}'`
MYGID=`\ls -ln $TESTDIR/test2.inc | awk '{print $4}'`
exec 2>&1
$TESTSUDOERS -U $MYUID -G $MYGID root id <<EOF
#include $TESTDIR/test2.inc
EOF

exit 0
