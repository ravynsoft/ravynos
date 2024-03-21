#!/bin/sh

# Test in the POSIX locale.
LC_ALL=C     ${CHECKER} ./test-iswpunct${EXEEXT} 0 || exit 1
LC_ALL=POSIX ${CHECKER} ./test-iswpunct${EXEEXT} 0 || exit 1

exit 0
