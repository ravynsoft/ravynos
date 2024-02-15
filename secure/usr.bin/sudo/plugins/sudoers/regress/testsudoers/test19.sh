#!/bin/sh
#
# Verify that "" in sudoers does not match a literal "" on the command line.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# This should succeed
$TESTSUDOERS root /bin/ls <<'EOF'
root ALL = /bin/ls ""
EOF

# This should fail
$TESTSUDOERS root /bin/ls '""' <<'EOF'
root ALL = /bin/ls ""
EOF

exit 0
