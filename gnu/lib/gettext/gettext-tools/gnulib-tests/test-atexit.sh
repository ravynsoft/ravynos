#!/bin/sh
: "${srcdir=.}"
. "$srcdir/init.sh"; path_prepend_ .

# Check that an atexit handler is called when main() returns normally.
echo > t-atexit.tmp
${CHECKER} test-atexit
if test -f t-atexit.tmp; then
  Exit 1
fi

# Check that an atexit handler is called when the program is left
# through exit(0).
echo > t-atexit.tmp
${CHECKER} test-atexit 0
if test -f t-atexit.tmp; then
  Exit 1
fi

# Check that an atexit handler is called when the program is left
# through exit(1).
echo > t-atexit.tmp
${CHECKER} test-atexit 1
if test -f t-atexit.tmp; then
  Exit 1
fi

Exit 0
