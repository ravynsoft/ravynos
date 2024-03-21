#!/bin/sh
#
# Test parsing of NOTBEFORE/NOTAFTER using local time zone
#

: ${VISUDO=visudo}

$VISUDO -cf - <<-EOF
	user1	ALL = NOTBEFORE=20151201235900 /usr/bin/id
	user2	ALL = NOTBEFORE=20151201235900.2 /usr/bin/id
	user3	ALL = NOTBEFORE=20151201235900\,2 /usr/bin/id
	user4	ALL = NOTBEFORE=2015120123 /usr/bin/id
	EOF
