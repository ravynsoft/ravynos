#!/bin/sh

# Execute the test only with seekable input stream.
# The behaviour of fflush() on a non-seekable input stream is undefined.
${CHECKER} ./test-fflush2${EXEEXT} 1 < "$srcdir/test-fflush2.sh" || exit $?
${CHECKER} ./test-fflush2${EXEEXT} 2 < "$srcdir/test-fflush2.sh" || exit $?
#cat "$srcdir/test-fflush2.sh" | ${CHECKER} ./test-fflush2${EXEEXT} || exit $?

exit 0
