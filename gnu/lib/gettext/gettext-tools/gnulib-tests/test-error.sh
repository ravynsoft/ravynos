#!/bin/sh
# Test of the 'error' module.

. "${srcdir=.}/init.sh"; path_prepend_ .

${CHECKER} test-error${EXEEXT} > out 2> err
# Verify the exit code.
case $? in
  4) ;;
  *) Exit 1;;
esac

# Normalize the stderr output on Windows platforms.
tr -d '\015' < err | sed 's,.*test-error[.ex]*:,test-error:,' > err2 || Exit 1

# Verify the stderr output.
compare - err2 <<\EOF || Exit 1
test-error: bummer
test-error: Zonk 123 is too large
test-error: Pokémon started
test-error:d1/foo.c:10: invalid blub
test-error:d1/foo.c:10: invalid blarn
test-error:d1/foo.c:10: unsupported glink
test-error:d1/foo.c:13: invalid brump
test-error:d2/foo.c:13: unsupported flinge
hammer
boing 123 is too large
d2/bar.c:11: bark too loud
test-error: can't steal: Permission denied
test-error: fatal error
EOF

# Verify the stdout output.
test -s out && Exit 1

Exit 0
