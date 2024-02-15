#!/bin/sh
#
# Test entries with no trailing newline.
#

: ${TESTSUDOERS=testsudoers}

exec 2>&1

echo ""
echo "Testing user privilege without a newline"
echo ""
printf "millert ALL = ALL" | $TESTSUDOERS -d

echo ""
echo "Testing alias without a newline"
echo ""
printf "Cmnd_Alias FOO=/bin/bar" | $TESTSUDOERS -d

echo ""
echo "Testing Defaults without a newline"
echo ""
printf "Defaults log_output" | $TESTSUDOERS -d

exit 0
