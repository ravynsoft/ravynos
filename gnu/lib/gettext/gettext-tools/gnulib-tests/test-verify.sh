#!/bin/sh
. "${srcdir=.}/init.sh"

# We are not interested in triggering bugs in the compilers and tools
# (such as gcc 4.3.1 on openSUSE 11.0).
unset MALLOC_PERTURB_

# Rather than figure out how to invoke the compiler with the right
# include path ourselves, we let make do it:
(cd "$initial_cwd_" \
 && rm -f test-verify-try.o \
 && $MAKE test-verify-try.o >/dev/null 2>&1) \
  || skip_ "cannot compile error-free"

# Now, prove that we encounter all expected compilation failures:
: >out
: >err
for i in 1 2 3 4 5; do
  (cd "$initial_cwd_"
   rm -f test-verify-try.o
   $MAKE CFLAGS=-DEXP_FAIL=$i test-verify-try.o) >>out 2>>err \
  && { warn_ "compiler didn't detect verification failure $i"; fail=1; }
done

Exit $fail
