#!/bin/sh

${CHECKER} ./test-ftello${EXEEXT} 1 < "$srcdir/test-ftello.sh" || exit 1
echo hi | ${CHECKER} ./test-ftello${EXEEXT} || exit 1
exit 0
