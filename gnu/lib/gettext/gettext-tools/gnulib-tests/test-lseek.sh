#!/bin/sh

tmpfiles=
trap 'rm -fr $tmpfiles' HUP INT QUIT TERM

tmpfiles=t-lseek.tmp
# seekable files
${CHECKER} ./test-lseek${EXEEXT} 0 < "$srcdir/test-lseek.sh" > t-lseek.tmp || exit 1

# pipes
echo hi | { ${CHECKER} ./test-lseek${EXEEXT} 1; echo $? > t-lseek.tmp; cat > /dev/null; } | cat
test "`cat t-lseek.tmp`" = "0" || exit 1

# closed descriptors
${CHECKER} ./test-lseek${EXEEXT} 2 <&- >&- || exit 1

rm -rf $tmpfiles
exit 0
