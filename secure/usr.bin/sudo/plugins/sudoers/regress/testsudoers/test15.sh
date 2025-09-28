#!/bin/sh
#
# Test @include of a file with a missing newline
#

: ${TESTSUDOERS=testsudoers}

# Create test file
TESTDIR="`pwd`/regress/testsudoers"
printf "root ALL = ALL" >"$TESTDIR/test15.inc"

MYUID=`\ls -lnd "$TESTDIR/test15.inc" | awk '{print $3}'`
MYGID=`\ls -lnd "$TESTDIR/test15.inc" | awk '{print $4}'`
exec 2>&1

echo "Testing @include of a file with a missing newline"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	@include $TESTDIR/test15.inc
	ALL ALL = /usr/bin/id
EOF

rm -f "$TESTDIR/test15.inc"
exit 0
