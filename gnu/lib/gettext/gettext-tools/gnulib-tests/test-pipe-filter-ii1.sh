#!/bin/sh

# Find a 'tr' program that supports character ranges in the POSIX syntax.
# Solaris /usr/bin/tr does not.
if test -f /usr/xpg6/bin/tr; then
  TR=/usr/xpg6/bin/tr
else
  if test -f /usr/xpg4/bin/tr; then
    TR=/usr/xpg4/bin/tr
  else
    TR=tr
  fi
fi

# A small file.
${CHECKER} ./test-pipe-filter-ii1${EXEEXT} ${TR} "${srcdir}/test-pipe-filter-ii1.sh" || exit 1
# A medium-sized file.
${CHECKER} ./test-pipe-filter-ii1${EXEEXT} ${TR} "${srcdir}/test-pipe-filter-ii1.c" || exit 1
# A large file.
${CHECKER} ./test-pipe-filter-ii1${EXEEXT} ${TR} "${srcdir}/test-vasnprintf-posix.c" || exit 1
