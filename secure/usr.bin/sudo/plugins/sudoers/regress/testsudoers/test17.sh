#!/bin/sh
#
# Test that digest matching works with LDAP sudoCommand: ALL
#

: ${TESTSUDOERS=testsudoers}

# Create test command with known digest
TESTDIR="`pwd`/regress/testsudoers"
cat >"$TESTDIR/hello" <<EOF
#!/bin/sh
echo Hello World
EOF
chmod 755 "$TESTDIR/hello"
SHA224_DIGEST="fIoq2MAfM/PZKTbkn9RE4VZ8YHjwnwTgE28Hxw=="

$TESTSUDOERS -i ldif root "${TESTDIR}/hello" <<-EOF
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
sudoCommand: sha224:$SHA224_DIGEST ALL
sudoOrder: 10
EOF

rm -f "$TESTDIR/hello"
exit 0
