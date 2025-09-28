#!/bin/sh
#
# Verify that runas_check_shell works as expected.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# This should fail due to fakeshell's shell
$TESTSUDOERS -u fakeshell -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
Defaults runas_check_shell
admin ALL = (ALL) /bin/ls
EOF

# Expected failure
if [ $? -eq 0 ]; then
    exit 1
else
    exit 0
fi
