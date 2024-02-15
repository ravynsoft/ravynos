#!/bin/sh
#
# Test user-specified chroot handling
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1
cd /

retval=0

printf "A simple sudoers rule should not allow the user to chroot:\n"
$TESTSUDOERS -R / root /bin/ls <<'EOF'
root ALL = /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

# Because command_matches() uses the per-rule CHROOT, this results in
# an unmatched rule instead of a matched rule that is rejected later.
# This is different from the CWD checking which is performed after
# matching is done.
printf "\nUser cannot override the sudoers chroot:\n"
$TESTSUDOERS -R / root /bin/ls <<'EOF'
root ALL = CHROOT=/some/where/else /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

printf "\nUser can chroot if sudoers rule sets chroot to '*':\n"
$TESTSUDOERS -R /usr root /bin/ls <<'EOF'
root ALL = CHROOT=* /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

printf "\nUser can chroot if runchroot Defaults is '*':\n"
$TESTSUDOERS -R /usr root /bin/ls <<'EOF'
Defaults runchroot = "*"
root ALL = /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

exit $retval
