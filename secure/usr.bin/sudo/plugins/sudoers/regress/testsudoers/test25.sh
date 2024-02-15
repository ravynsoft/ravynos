#!/bin/sh
#
# Test user-specified cwd handling
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1
cd /

retval=0

# Sudo used to allow the user to set the cwd to the current value.
# Now, a cwd must be explicitly set in sudoers to use the -D option.
printf "A simple sudoers rule should not allow the user to set the cwd:\n"
$TESTSUDOERS -D / root /bin/ls <<'EOF'
root ALL = /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

printf "\nUser cannot override the sudoers cwd:\n"
$TESTSUDOERS -D / root /bin/ls <<'EOF'
root ALL = CWD=/some/where/else /bin/ls
EOF
if [ $? -eq 0 ]; then
    retval=1
fi

printf "\nUser can set cwd if sudoers rule sets cwd to '*':\n"
$TESTSUDOERS -D /usr root /bin/ls <<'EOF'
root ALL = CWD=* /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

printf "\nUser can set cwd runcwd Defaults is '*':\n"
$TESTSUDOERS -D /usr root /bin/ls <<'EOF'
Defaults runcwd = "*"
root ALL = /bin/ls
EOF
if [ $? -ne 0 ]; then
    retval=$?
fi

exit $retval
