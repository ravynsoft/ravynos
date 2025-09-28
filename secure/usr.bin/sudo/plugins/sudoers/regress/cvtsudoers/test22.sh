#!/bin/sh
#
# Test LDAP base filtering.
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -i ldif -b "ou=SUDOers,dc=sudo,dc=ws" -I 10 -O 10 <<EOF
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

# root, SUDOers, sudo.ws
dn: cn=root,ou=SUDOers,dc=sudo,dc=ws
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
dn: cn=%wheel,ou=SUDOers,dc=sudo,dc=ws
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
dn: cn=millert,ou=SUDOers,dc=other-domain,dc=com
objectClass: top
objectClass: sudoRole
cn: millert
sudoUser: millert
sudoRunAsUser: ALL
sudoRunAsGroup: ALL
sudoHost: ALL
sudoOrder: 5
EOF
