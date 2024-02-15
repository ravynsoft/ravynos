#!/bin/sh
#
# Test LDAP sudoOrder when converting to sudoers.
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i ldif -b "ou=SUDOers,dc=sudo,dc=ws" -f sudoers <<EOF
dn: dc=sudo,dc=ws
objectClass: dcObject
objectClass: organization
dc: courtesan
o: Sudo World Headquarters
description: Sudo World Headquarters

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

# defaults, SUDOers, sudo.ws
dn: cn=defaults,ou=SUDOers,dc=sudo,dc=ws
objectClass: top
objectClass: sudoRole
cn: defaults
description: Default sudoOption's go here
sudoOption: log_output

# millert, SUDOers, sudo.ws
dn: cn=millert,ou=SUDOers,dc=sudo,dc=ws
objectClass: top
objectClass: sudoRole
cn: millert
sudoUser: millert
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: ALL
sudoCommand: ALL
sudoOrder: 5

# millert2, SUDOers, sudo.ws
dn: cn=millert2,ou=SUDOers,dc=sudo,dc=ws
objectClass: top
objectClass: sudoRole
cn: millert2
sudoUser: millert
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: ALL
sudoCommand: ALL
sudoOption: !authenticate
sudoOrder: 10
EOF
