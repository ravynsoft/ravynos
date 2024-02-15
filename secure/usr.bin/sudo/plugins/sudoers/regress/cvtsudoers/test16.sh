#!/bin/sh
#
# Test filters and pruning
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -f sudoers -p -m user=user2,host=host2 <<EOF
user1, user2, user3, %group1 host1, host2, host3 = ALL
EOF
