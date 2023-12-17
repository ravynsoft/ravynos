#!/bin/sh

exec ${CHECKER} ./test-fseeko${EXEEXT} 1 2 < "$srcdir/test-fseeko2.sh"
