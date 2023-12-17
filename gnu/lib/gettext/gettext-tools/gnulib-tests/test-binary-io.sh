#!/bin/sh

tmpfiles=""
trap 'rm -fr $tmpfiles' HUP INT QUIT TERM

tmpfiles="$tmpfiles t-bin-out0.tmp t-bin-out1.tmp"
${CHECKER} ./test-binary-io${EXEEXT} 1 > t-bin-out1.tmp || exit 1
cmp t-bin-out0.tmp t-bin-out1.tmp > /dev/null || exit 1

rm -fr $tmpfiles

exit 0
