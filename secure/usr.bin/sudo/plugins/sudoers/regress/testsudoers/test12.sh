#!/bin/sh
#
# Test sudoers file with multiple syntax errors
# The standard error output is dup'd to the standard output.
#

: ${TESTSUDOERS=testsudoers}

echo "Testing sudoers with multiple syntax errors"
echo ""
$TESTSUDOERS -d <<EOF 2>&1 | sed 's/\(syntax error\), .*/\1/' 
User_Alias A1 = u1 u2 : A2 = u3, u4

millert ALL = /fail : foo

root ALL = ALL bar

root ALL = baz
EOF
