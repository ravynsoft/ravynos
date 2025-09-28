#!/bin/sh
#
# Test LDIF base64 attribute parsing
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i ldif -b "ou=SUDOers,dc=sudo,dc=ws" -I 10 -O 10 <<EOF
# defaults, SUDOers, sudo.ws
dn:: Y249ZGVmYXVsdHMsb3U9U1VET2VycyxkYz1zdWRvLGRjPXdz 
objectClass: top
objectClass: sudoRole
cn: defaults
description: Default sudoOption's go here
sudoOption:: bG9nX291dHB1dA== 

# root, SUDOers, sudo.ws
dn::  Y249cm9vdCxvdT1TVURPZXJzLGRjPXN1ZG8sZGM9d3M=
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
dn:: Y249JXdoZWVsLG91PVNVRE9lcnMsZGM9c3VkbyxkYz13cw==
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

# millert, SUDOers, other-domain.com
dn:: Y249bWlsbGVydCxvdT1TVURPZXJzLGRjPW90aGVyLWRvbWFpbixkYz1jb20=
objectClass: top
objectClass: sudoRole
cn: millert
sudoUser: millert
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: ALL
sudoOrder: 5
EOF
