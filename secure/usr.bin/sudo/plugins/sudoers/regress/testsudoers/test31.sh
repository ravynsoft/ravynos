#!/bin/sh
#
# Exercise "sudo -U user -l [command]"
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

status=0

echo "'sudo -U root -l' with no matching rules"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
root ALL = ALL
EOF

echo ""
echo "'sudo -U root -l' with a matching ALL=ALL rule"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
admin ALL = ALL
EOF

echo ""
echo "'sudo -U root -l' with a matching list rule"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
admin ALL = NOPASSWD: list
EOF

echo ""
echo "'sudo -U root -l' without a matching list rule"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
admin ALL = (operator) list
EOF

echo ""
echo "'sudo -U root -l' with a negated list rule"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
admin ALL = !list
EOF

echo ""
echo "'sudo -U root -l' with a list rule that is later negated"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin <<'EOF'
admin ALL = NOPASSWD: list, !list
EOF

echo ""
echo "'sudo -l command' with a matching command"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin /bin/ls <<'EOF'
admin ALL = /bin/ls
EOF

echo ""
echo "'sudo -l command' without a matching command"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -l admin /usr/bin/id <<'EOF'
admin ALL = /bin/ls
EOF

echo ""
echo "'sudo -U root -l command' without list privileges"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin /bin/ls <<'EOF'
root ALL = ALL
admin ALL = /usr/bin/id
EOF

echo ""
echo "'sudo -U root -l command' with list privileges"
$TESTSUDOERS -p ${TESTDIR}/passwd -P ${TESTDIR}/group -L root admin /bin/ls <<'EOF'
root ALL = ALL
admin ALL = list
EOF
