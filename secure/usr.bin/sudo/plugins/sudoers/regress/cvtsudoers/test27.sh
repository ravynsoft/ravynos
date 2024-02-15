#!/bin/sh
#
# Test base64 encoding of non-safe strings
#

: ${CVTSUDOERS=cvtsudoers}

$CVTSUDOERS -c "" -b "ou=SUDOers©,dc=sudo,dc=ws" <<EOF
Defaults badpass_message="Bad password¡"

root ALL = ALL
EOF
