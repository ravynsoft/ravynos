#!/bin/sh
#
# Sudo Bug 519:
# Visudo in strict mode reports "parse error" even if there is no error
#

: ${VISUDO=visudo}

$VISUDO -csf - <<EOF
User_Alias FOO = nobody
FOO ALL=(ALL) NOPASSWD: ALL
EOF

exit 0
