#!/bin/sh
#
# Verify that NOTBEFORE and NOTAFTER work as expected.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

retval=0

$TESTSUDOERS -T 20170214083000Z root /bin/ls <<'EOF'
root ALL = NOTBEFORE=20170214083000Z /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

# expect failure
$TESTSUDOERS -T 20170214083000Z root /bin/ls <<'EOF'
root ALL = NOTBEFORE=20170214083001Z /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

$TESTSUDOERS -T 20170214083000Z root /bin/ls <<'EOF'
root ALL = NOTAFTER=20170214083000Z /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

# expect failure
$TESTSUDOERS -T 20170214083001Z root /bin/ls <<'EOF'
root ALL = NOTAFTER=20170214083000Z /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

exit $retval
