#!/bin/sh
#
# Test @includedir facility
#

: ${TESTSUDOERS=testsudoers}

TESTDIR="`pwd`/regress/testsudoers"
# make sure include file is owned by current user
rm -rf "$TESTDIR/test3.d"
mkdir "$TESTDIR/test3.d"
cat >"$TESTDIR/test3.d/root" <<-EOF
	root ALL = ALL
EOF

MYUID=`\ls -lnd $TESTDIR/test3.d | awk '{print $3}'`
MYGID=`\ls -lnd $TESTDIR/test3.d | awk '{print $4}'`
exec 2>&1

echo "Testing @includedir of an unquoted path"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	@includedir $TESTDIR/test3.d
EOF

echo ""
echo "Testing @includedir of a double-quoted path"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	@includedir "$TESTDIR/test3.d"
EOF

echo ""
echo "Testing #includedir of an unquoted path"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	#includedir $TESTDIR/test3.d
EOF

echo ""
echo "Testing #includedir of a double-quoted path"
echo ""
$TESTSUDOERS -U $MYUID -G $MYGID root id <<-EOF
	#includedir "$TESTDIR/test3.d"
EOF

rm -rf "$TESTDIR/test3.d"
exit 0
