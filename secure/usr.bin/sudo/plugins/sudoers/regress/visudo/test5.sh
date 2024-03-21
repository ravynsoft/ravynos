#!/bin/sh
#
# Test comment on the last line with no newline
#

: ${VISUDO=visudo}

printf "# one comment\n#two comments" | $VISUDO -csf -

exit 0
