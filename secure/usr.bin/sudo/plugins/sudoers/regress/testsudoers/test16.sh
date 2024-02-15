#!/bin/sh
#
# Test to exercise Bug #994, a crash matching sudoCommand ALL.
#

: ${TESTSUDOERS=testsudoers}

$TESTSUDOERS -i ldif root id <<-EOF
dn: dc=sudo,dc=ws
objectClass: dcObject
objectClass: organization
dc: bigwheel
o: Big Wheel
description: Big Wheel

# Organizational Role for Directory Manager
dn: cn=Manager,dc=sudo,dc=ws
objectClass: organizationalRole
cn: Manager
description: Directory Manager

# SUDOers, sudo.ws
dn: ou=SUDOers,dc=sudo,dc=ws
objectClass: top
objectClass: organizationalUnit
description: SUDO Configuration Subtree
ou: SUDOers

# root, SUDOers, sudo.ws
dn: cn=root,ou=SUDOers,dc=sudo,dc=ws
objectClass: top
objectClass: sudoRole
cn: root
sudoUser: root
sudoRunAs: ALL
sudoHost: ALL
sudoCommand: ALL
sudoOrder: 10
EOF

exit 0
