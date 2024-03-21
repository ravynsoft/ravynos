#!/bin/sh

exec ${CHECKER} ./test-ftell${EXEEXT} 1 2 < "$srcdir/test-ftell2.sh"
