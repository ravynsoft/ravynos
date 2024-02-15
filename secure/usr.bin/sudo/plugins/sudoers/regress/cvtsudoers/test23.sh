#!/bin/sh
#
# Test round-tripping of sudoers -> LDIF -> sudoers
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -b "ou=SUDOers,dc=sudo,dc=ws" $TESTDIR/test23.out.ok | \
    $CVTSUDOERS -c "" -i LDIF -f sudoers | grep -v '^#'
