#!/bin/sh

${CHECKER} ./test-ftell${EXEEXT} 1 < "$srcdir/test-ftell.sh" || exit 1
echo hi | ${CHECKER} ./test-ftell${EXEEXT} || exit 1
exit 0
