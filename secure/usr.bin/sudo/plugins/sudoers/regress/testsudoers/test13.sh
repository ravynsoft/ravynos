#!/bin/sh
#
# Test sudoers file with reserved words as alias names.
# The standard error output is dup'd to the standard output.
#

: ${TESTSUDOERS=testsudoers}

echo "Testing alias definitions using reserved words"
echo ""
$TESTSUDOERS -d <<EOF 2>&1
Cmnd_Alias ALL=ALL
Cmnd_Alias CHROOT=foo
User_Alias TIMEOUT=foo
Runas_Alias CWD=bar
Host_Alias NOTBEFORE=baz
Host_Alias NOTAFTER=biff

root ALL = ALL
EOF

exit 0
