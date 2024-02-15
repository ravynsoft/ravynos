#!/bin/sh
#
# Test round-tripping of LDIF -> sudoers -> LDIF
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i LDIF -f sudoers $TESTDIR/test24.out.ok | \
    $CVTSUDOERS -c "" -b "ou=SUDOers,dc=sudo,dc=ws"
