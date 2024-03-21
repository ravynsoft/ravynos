#!/bin/sh

# Test in the POSIX locale.
LC_ALL=C \
${CHECKER} ./test-fnmatch${EXEEXT} 1 || exit 1
LC_ALL=POSIX \
${CHECKER} ./test-fnmatch${EXEEXT} 1 || exit 1

exit 0
