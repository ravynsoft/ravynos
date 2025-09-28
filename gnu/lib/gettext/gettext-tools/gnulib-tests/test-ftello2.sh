#!/bin/sh

exec ${CHECKER} ./test-ftello${EXEEXT} 1 2 < "$srcdir/test-ftello2.sh"
