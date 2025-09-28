#!/bin/sh
#
# Test @include of a file with embedded white space
#

: ${TESTSUDOERS=testsudoers}

# Create test file
TESTDIR="`pwd`/regress/testsudoers"
cat >"$TESTDIR/test 10.inc" <<EOF
root ALL = ALL
EOF

MYUID=`\ls -lnd "$TESTDIR/test 10.inc" | awk '{print $3}'`
MYGID=`\ls -lnd "$TESTDIR/test 10.inc" | awk '{print $4}'`
exec 2>&1

echo "Testing @include of a path with escaped white space"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	@include $TESTDIR/test\ 10.inc
EOF

echo ""
echo "Testing @include of a double-quoted path with white space"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	@include "$TESTDIR/test 10.inc"
EOF

echo ""
echo "Testing #include of a path with escaped white space"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	#include $TESTDIR/test\ 10.inc
EOF

echo ""
echo "Testing #include of a double-quoted path with white space"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	#include "$TESTDIR/test 10.inc"
EOF

rm -f "$TESTDIR/test 10.inc"
exit 0
