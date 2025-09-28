#!/bin/sh
#
# Test cycle detection
# Prior to sudo 1.8.6p5 this resulted in a core dump (stack smash)
# The names of the aliases (or rather their lexical order) is important.
#

: ${VISUDO=visudo}

$VISUDO -csf - <<EOF
User_Alias YYY = FOO
User_Alias XXX = nobody
User_Alias FOO = XXX, YYY
FOO ALL = ALL
EOF

exit 0
