#!/bin/sh
#
# Test use-after-free in cvtsudoers when filtering by command.
#
# If compiled with address sanitizer, cvtsudoers will crash without the
# fix in 9da99e0e671e.
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i ldif -b "ou=SUDOers,dc=sudo,dc=ws" -m cmd='/bin/ls' -p <<EOF
objectClass:sudoRole
sudoUser:user0
sudoHost:A00
sudoCommand:/bin/ls
sudoRunAs:0

objectClass:sudoRole
sudoUser:user0
sudoHost:A00
sudoRunAsUser:
sudoCommand:

objectClass:sudoRole
sudoUser:user0
sudoHost:A00
sudoRunAs:
sudoCommand:
EOF
