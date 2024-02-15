#!/bin/sh
#
# Verify sudoers matching by gid.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1
$TESTSUDOERS root id <<EOF
%#0 ALL = ALL
EOF

exit 0
