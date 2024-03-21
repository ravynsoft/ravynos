#!/bin/sh

tmpfiles=""
trap 'rm -fr $tmpfiles' HUP INT QUIT TERM

# Test signal's default behaviour.
tmpfiles="$tmpfiles t-sigpipeA.tmp"
${CHECKER} ./test-sigpipe${EXEEXT} A 2> t-sigpipeA.tmp | head -n 1 > /dev/null
if test -s t-sigpipeA.tmp; then
  LC_ALL=C tr -d '\r' < t-sigpipeA.tmp
  rm -fr $tmpfiles; exit 1
fi

# Test signal's ignored behaviour.
tmpfiles="$tmpfiles t-sigpipeB.tmp"
${CHECKER} ./test-sigpipe${EXEEXT} B 2> t-sigpipeB.tmp | head -n 1 > /dev/null
if test -s t-sigpipeB.tmp; then
  LC_ALL=C tr -d '\r' < t-sigpipeB.tmp
  rm -fr $tmpfiles; exit 1
fi

# Test signal's behaviour when a handler is installed.
tmpfiles="$tmpfiles t-sigpipeC.tmp"
${CHECKER} ./test-sigpipe${EXEEXT} C 2> t-sigpipeC.tmp | head -n 1 > /dev/null
if test -s t-sigpipeC.tmp; then
  LC_ALL=C tr -d '\r' < t-sigpipeC.tmp
  rm -fr $tmpfiles; exit 1
fi

rm -fr $tmpfiles
exit 0
