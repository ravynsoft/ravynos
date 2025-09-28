#!/bin/sh

${CHECKER} ./test-fseeko3${EXEEXT} 0 "$srcdir/test-fseeko3.sh" || exit 1

${CHECKER} ./test-fseeko3${EXEEXT} 1 "$srcdir/test-fseeko3.sh" || exit 1

exit 0
