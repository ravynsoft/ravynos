#!/bin/sh
#
# Test handling of a backslash at EOF with no trailing newline.
#
# If compiled with address sanitizer, cvtsudoers will crash without the
# fix in ceaf706ab74b from https://github.com/sudo-project/sudo/pull/196.
#

: ${CVTSUDOERS=cvtsudoers}

printf 'dn: cn= Manager\\' | \
    $CVTSUDOERS -c "" -b "ou=SUDOers,dc=sudo,dc=ws" -i ldif -f sudoers
