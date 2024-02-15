#!/bin/sh
#
# Verify that a rule with an empty Runas user matches correctly.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

status=0

echo "This should match the 'ALL=ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
admin ALL = ALL
ALL ALL=(:staff) NOPASSWD: ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should match the 'ALL=ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
ALL ALL=(:staff) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should match the 'ALL=(:staff) NOPASSWD: ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -g staff \
    admin /bin/ls <<'EOF'
admin ALL = ALL
ALL ALL=(:staff) NOPASSWD: ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should match the 'ALL=(:staff) NOPASSWD: ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -g staff \
    admin /bin/ls <<'EOF'
ALL ALL=(:staff) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should match the 'ALL=(:staff) NOPASSWD: ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -u admin \
    admin /bin/ls <<'EOF'
ALL ALL=(:staff) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should match the 'ALL=(:staff) NOPASSWD: ALL' rule."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -u admin -g staff \
    admin /bin/ls <<'EOF'
ALL ALL=(:staff) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -ne 0 ]; then
    status=1
fi

echo ""
echo "This should not match any rules."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -g guest \
    admin /bin/ls <<'EOF'
ALL ALL=(:staff) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -eq 0 ]; then
    status=1
fi

echo ""
echo "This should not match any rules."
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -u root -g users \
    admin /bin/ls <<'EOF'
ALL ALL=(:users) NOPASSWD: ALL
admin ALL = ALL
EOF
if [ $? -eq 0 ]; then
    status=1
fi

exit $status
