#!/bin/sh
#
# Exercise listpw Defaults settings.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

status=0

echo "listpw = all, 'sudo -l' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = all
admin ALL = NOPASSWD: ALL
admin ALL = /usr/bin/id
EOF

echo ""
echo "listpw = all, 'sudo -l' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = all
admin ALL = /usr/bin/id
admin ALL = NOPASSWD: ALL
EOF

echo ""
echo "listpw = all, 'sudo -l' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = all
admin ALL = NOPASSWD: ALL
admin ALL = NOPASSWD: /usr/bin/id
EOF

echo ""
echo "listpw = always, 'sudo -l' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = always
admin ALL = NOPASSWD: ALL
EOF

echo ""
echo "listpw = any, 'sudo -l' should require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = any
admin ALL = ALL
admin ALL = /usr/bin/id
EOF

echo ""
echo "listpw = any, 'sudo -l' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = any
admin ALL = ALL
admin ALL = NOPASSWD: /usr/bin/id
EOF

echo ""
echo "listpw = any, 'sudo -l' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = any
admin ALL = NOPASSWD: /usr/bin/id
admin ALL = ALL
EOF

echo ""
echo "listpw = never, 'sudo -l' should not require a password"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin <<'EOF'
Defaults listpw = never
admin ALL = PASSWD: /usr/bin/id
EOF
