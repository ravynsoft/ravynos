#!/bin/sh
#
# Verify that a user is not allowed to run commands with their own
# user and group if sudoers doesn't explicitly permit it.
# This tests a bug fixed in sudo 1.9.14.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

$TESTSUDOERS -u admin -g admin -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
admin ALL = (root) /bin/ls
EOF

exit 0
