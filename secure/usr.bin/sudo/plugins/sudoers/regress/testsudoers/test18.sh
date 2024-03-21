#!/bin/sh
#
# Test regular expressions
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

# Command and args: regex
$TESTSUDOERS root /bin/ls -l <<'EOF'
root ALL = ^/bin/ls$ ^-[lAt]$
EOF

# Command: regex, args: wildcard
$TESTSUDOERS root /bin/cat /var/log/syslog <<'EOF'
root ALL = ^/bin/cat$ /var/log/*
EOF

# Command: path, args: regex
$TESTSUDOERS root /bin/cat /var/log/authlog <<'EOF'
root ALL = /bin/cat ^/var/log/[^/]+$
EOF

# Command: wildcard, args: regex
$TESTSUDOERS root /bin/cat /var/log/mail <<'EOF'
root ALL = /bin/*at ^/var/log/[^/]+$
EOF

# Command: path, args: args start with escaped ^
$TESTSUDOERS root /usr/bin/grep '^foo$' <<'EOF'
root ALL = /usr/bin/grep \^foo$
EOF

# Command: sudoedit, args: regex
$TESTSUDOERS root sudoedit /etc/motd <<'EOF'
root ALL = sudoedit ^/etc/(motd|issue|hosts)$
EOF

exit 0
