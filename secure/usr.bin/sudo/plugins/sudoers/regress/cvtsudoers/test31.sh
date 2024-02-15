#!/bin/sh
#
# Test cvtsudoers.conf with padding
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c $TESTDIR/test31.conf <<EOF
Defaults authenticate, timestamp_timeout=0
User_Alias FULLTIMERS = user1, user2, user3

ALL ALL = (:) NOPASSWD:/usr/bin/id
FULLTIMERS ALL = (ALL:ALL) ALL
EOF
