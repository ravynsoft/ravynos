#!/bin/sh
#
# Test cvtsudoers.conf with invalid padding
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -b "ou=SUDOers,dc=my-domain,dc=com" -O 1000 -P 1 <<EOF
user0  ALL = (ALL:ALL) ALL
user1  ALL = (ALL:ALL) ALL
user2  ALL = (ALL:ALL) ALL
user3  ALL = (ALL:ALL) ALL
user4  ALL = (ALL:ALL) ALL
user5  ALL = (ALL:ALL) ALL
user6  ALL = (ALL:ALL) ALL
user7  ALL = (ALL:ALL) ALL
user8  ALL = (ALL:ALL) ALL
user9  ALL = (ALL:ALL) ALL
user10 ALL = (ALL:ALL) ALL
EOF

exit 0
