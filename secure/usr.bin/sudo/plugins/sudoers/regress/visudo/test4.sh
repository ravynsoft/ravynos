#!/bin/sh
#
# Test cycle detection and duplicate entries.
# Prior to sudo 1.8.7 this resulted in a false positive.
#

: ${VISUDO=visudo}

$VISUDO -csf - <<EOF
Host_Alias H1 = host1
Host_Alias H2 = H1, host2
Host_Alias H3 = H1, H2
root H3 = ALL
EOF

exit 0
