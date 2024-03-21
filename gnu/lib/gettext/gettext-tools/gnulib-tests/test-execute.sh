#!/bin/sh

st=0
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 ; do
  ${CHECKER} ./test-execute-main${EXEEXT} ./test-execute-child${EXEEXT} $i \
    || { echo test-execute.sh: test case $i failed >&2; st=1; }
done
exit $st
