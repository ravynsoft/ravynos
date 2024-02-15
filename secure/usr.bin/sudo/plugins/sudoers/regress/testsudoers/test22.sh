#!/bin/sh
#
# Verify that a user is only allowed to run commands with a group
# that is specified by sudoers (or that the runas user is a member of).
# This tests a bug fixed in sudo 1.9.14.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# The root user must *not* belong to the group specified below.
$TESTSUDOERS -u root -g bin -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
admin ALL = /bin/ls
EOF

exit 0
