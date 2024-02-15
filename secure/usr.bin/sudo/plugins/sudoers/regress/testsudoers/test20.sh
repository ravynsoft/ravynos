#!/bin/sh
#
# Verify CHROOT and CWD support
# This will catch an unpatched double-free in set_cmnd_path() under ASAN.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# Exercise double free of user_cmnd in set_cmnd_path() under ASAN.
# We need more than one rule where the last rule matches and has CHROOT.
$TESTSUDOERS root /bin/ls <<'EOF'
root ALL = CWD=/ /bin/pwd
root ALL = CHROOT=/ /bin/ls
EOF

exit 0
