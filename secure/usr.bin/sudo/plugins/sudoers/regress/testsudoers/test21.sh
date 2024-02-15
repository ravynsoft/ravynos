#!/bin/sh
#
# Verify that a Runas_Alias works in both user and group lists.
# This tests a bug fixed in sudo 1.9.14.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# The user in USERALIAS must *not* belong to the group in GROUPALIAS
# in the group or passwd file in order to reproduce the bug.
$TESTSUDOERS -u root -g bin -p ${TESTDIR}/passwd -P ${TESTDIR}/group \
    admin /bin/ls <<'EOF'
Runas_Alias USERALIAS = root
Runas_Alias GROUPALIAS = bin
admin ALL = (USERALIAS : GROUPALIAS) /bin/ls
EOF

exit 0
