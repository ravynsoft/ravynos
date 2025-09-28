#!/bin/sh
# Test select() on file descriptors opened for writing.

tmpfiles=""
trap 'rm -fr $tmpfiles' HUP INT QUIT TERM

tmpfiles="$tmpfiles t-select-out.out t-select-out.tmp"

# Regular files.

rm -f t-select-out.tmp
${CHECKER} ./test-select-fd${EXEEXT} w 1 t-select-out.tmp > t-select-out.out
test `cat t-select-out.tmp` = "1" || exit 1

# Pipes.

if false; then # This test fails on some platforms.
  rm -f t-select-out.tmp
  ( { echo abc; ${CHECKER} ./test-select-fd${EXEEXT} w 1 t-select-out.tmp; } | { sleep 1; cat; } ) > /dev/null
  test `cat t-select-out.tmp` = "0" || exit 1
fi

rm -f t-select-out.tmp
( { sleep 1; echo abc; ${CHECKER} ./test-select-fd${EXEEXT} w 1 t-select-out.tmp; } | cat) > /dev/null
test `cat t-select-out.tmp` = "1" || exit 1

# Special files.

rm -f t-select-out.tmp
${CHECKER} ./test-select-fd${EXEEXT} w 1 t-select-out.tmp > /dev/null
test `cat t-select-out.tmp` = "1" || exit 1

rm -fr $tmpfiles

exit 0
