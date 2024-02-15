#!/bin/sh
#
# Test IP and network address in host-based Defaults statements
# Bugzilla #766
#

: ${VISUDO=visudo}

$VISUDO -cf - <<-EOF
	Defaults@127.0.0.1 !authenticate
	Defaults@10.0.0.0/8 !always_set_home
	EOF

exit 0
