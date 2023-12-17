#!/bin/sh
. "${srcdir=.}/init.sh"; path_prepend_ .

${CHECKER} test-string-desc${EXEEXT} test-string-desc-3.tmp > test-string-desc-1.tmp || Exit 1

printf 'Hello world!The\0quick\0brown\0\0fox\0' > test-string-desc.ok

: "${DIFF=diff}"
${DIFF} test-string-desc.ok test-string-desc-1.tmp || { echo "string_desc_fwrite KO" 1>&2; Exit 1; }
${DIFF} test-string-desc.ok test-string-desc-3.tmp || { echo "string_desc_write KO" 1>&2; Exit 1; }

Exit 0
