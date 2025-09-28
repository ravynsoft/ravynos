#!/bin/sh
exec ${CHECKER} ./test-uninames${EXEEXT} "$srcdir/uniname/UnicodeData.txt" "$srcdir/uniname/HangulSyllableNames.txt" -- "$srcdir/uniname/NameAliases.txt"
