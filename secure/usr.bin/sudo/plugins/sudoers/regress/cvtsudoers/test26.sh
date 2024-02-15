#!/bin/sh
#
# Test LDIF invalid base64 attribute parsing
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i ldif -b "ou=SUDOers,dc=sudo,dc=ws" -I 10 -O 10 <<EOF
# defaults, SUDOers, sudo.ws
dn:: Y249ZGVmYXVsdHMsb3U9U1VET2VycyxkYz1zdWRvLGRjPXdz 
objectClass: top
objectClass: sudoRole
cn: defaults
description: Default sudoOption's go here
sudoOption:: bG9nX29@1dHB1dA== 

# root, SUDOers, sudo.ws
dn::  Y249cm9vdCxvdT1TVURPZXJzLGRjPXN1ZG8sZGM9_d3M=
objectClass: top
objectClass: sudoRole
cn: root
sudoUser: root
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: ALL
sudoCommand: ALL
sudoOption: !authenticate
sudoOrder: 10

# %wheel, SUDOers, sudo.ws
dn:: Y249JXdoZWVsLG91PVNVRE9lcnMsZGM9c3VkbyxkYz13cw!==
objectClass: top
objectClass: sudoRole
cn: %wheel
sudoUser: %wheel
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: +sudo-hosts
sudoCommand: ALL
sudoOption: !authenticate
sudoOrder: 10
EOF

# cvtsudoers should exit with an error
if [ $? -eq 0 ]; then
    exit 1
else
    exit 0
fi
