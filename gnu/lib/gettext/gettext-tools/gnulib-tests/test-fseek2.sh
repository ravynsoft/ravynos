#!/bin/sh

exec ${CHECKER} ./test-fseek${EXEEXT} 1 2 < "$srcdir/test-fseek2.sh"
