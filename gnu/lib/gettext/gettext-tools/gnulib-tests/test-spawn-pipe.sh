#!/bin/sh

st=0
for i in 0 1 2 3 4 5 6 7 ; do
  ${CHECKER} ./test-spawn-pipe-main${EXEEXT} ./test-spawn-pipe-child${EXEEXT} $i \
    || { echo test-spawn-pipe.sh: iteration $i failed >&2; st=1; }
done
exit $st
