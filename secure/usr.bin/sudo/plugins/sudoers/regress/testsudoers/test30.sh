#!/bin/sh
#
# Exercise verifypw Defaults settings.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

status=0

echo "verifypw = all, 'sudo -v' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = all
admin ALL = NOPASSWD: ALL
admin ALL = /usr/bin/id
EOF

echo ""
echo "verifypw = all, 'sudo -v' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = all
admin ALL = /usr/bin/id
admin ALL = NOPASSWD: ALL
EOF

echo ""
echo "verifypw = all, 'sudo -v' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = all
admin ALL = NOPASSWD: ALL
admin ALL = NOPASSWD: /usr/bin/id
EOF

echo ""
echo "verifypw = always, 'sudo -v' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = always
admin ALL = NOPASSWD: ALL
EOF

echo ""
echo "verifypw = any, 'sudo -v' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = any
admin ALL = ALL
admin ALL = /usr/bin/id
EOF

echo ""
echo "verifypw = any, 'sudo -v' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = any
admin ALL = ALL
admin ALL = NOPASSWD: /usr/bin/id
EOF

echo ""
echo "verifypw = any, 'sudo -v' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = any
admin ALL = NOPASSWD: /usr/bin/id
admin ALL = ALL
EOF

echo ""
echo "verifypw = never, 'sudo -v' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -v admin <<'EOF'
Defaults verifypw = never
admin ALL = PASSWD: /usr/bin/id
EOF
