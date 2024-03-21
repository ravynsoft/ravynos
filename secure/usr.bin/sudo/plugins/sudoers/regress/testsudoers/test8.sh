#!/bin/sh
#
# Test @include facility w/o a final newline.
# Same as test2.sh but missing the final newline.
#

: ${TESTSUDOERS=testsudoers}

MYUID=`\ls -ln $TESTDIR/test2.inc | awk '{print $3}'`
MYGID=`\ls -ln $TESTDIR/test2.inc | awk '{print $4}'`
exec 2>&1

echo "Testing @include without a newline"
echo ""
printf "@include $TESTDIR/test2.inc" | \
    $TESTSUDOERS -U $MYUID -G $MYGID root id

echo ""
echo "Testing #include without a newline"
echo ""
printf "#include $TESTDIR/test2.inc" | \
    $TESTSUDOERS -U $MYUID -G $MYGID root id

exit 0
