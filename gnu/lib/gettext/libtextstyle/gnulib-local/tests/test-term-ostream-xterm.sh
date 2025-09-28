#!/bin/sh

tmpfiles=""
trap 'rm -fr $tmpfiles' HUP INT QUIT TERM

tmpfiles="$tmpfiles out1 out"
# The redirection of stderr into a pipe avoids the output of padding bytes
# (unnecessary NUL bytes after escape sequences) on some systems.
(TERM=xterm ./test-term-ostream > out1) 2>&1 | cat 1>&2
LC_ALL=C tr -d '\r' < out1 > out

# There are several variants of the "xterm" terminal description floating
# around, each with a different sgr0 escape sequence. Use "infocmp -l -1 xterm"
# to inspect the escape sequences of xterm on your platform.
#   xterm-r6:             sgr0=\E[m
#   xterm-xf86-v32:       sgr0=\E[m\017
#   xterm-linux-mandriva: sgr0=\E[m\E(B
#   xterm-basic:          sgr0=\E(B\E[m
#   xterm-8bit:           sgr0=\2330m\E(B

: ${DIFF=diff}
   ${DIFF} ${srcdir}/test-term-ostream-xterm-r6.out             out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-xf86-v32.out       out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-basic.out          out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-basic-italic.out   out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-freebsd101.out     out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-freebsd-italic.out out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-8bit.out           out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-linux-debian.out   out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-linux-mandriva.out out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-netbsd3.out        out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-solaris10.out      out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-aix51.out          out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-osf51.out          out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-irix65.out         out > /dev/null \
|| ${DIFF} ${srcdir}/test-term-ostream-xterm-mingw.out          out > /dev/null
result=$?

rm -fr $tmpfiles

exit $result
