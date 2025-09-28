#!/bin/sh
echo >&2 You need to edit this to run your test case
exit 1

git clean -dxf

# If you get './makedepend: 1: Syntax error: Unterminated quoted
# string' when bisecting versions of perl older than 5.9.5 this hack
# will work around the bug in makedepend.SH which was fixed in
# version 96a8704c. Make sure to uncomment 'git checkout makedepend.SH'
# below too.
#git show blead:makedepend.SH > makedepend.SH

# If you can use ccache, add -Dcc=ccache\ gcc -Dld=gcc to the Configure line
# if Encode is not needed for the test, you can speed up the bisect by
# excluding it from the runs with -Dnoextensions=Encode
# ie
#./Configure -Dusedevel -Doptimize=-g -Dcc=ccache\ gcc -Dld=gcc -Dnoextensions=Encode -des
./Configure -Dusedevel -Doptimize=-g -des
test -f config.sh || exit 125
# Correct makefile for newer GNU gcc
perl -ni -we 'print unless /<(?:built-in|command)/' makefile x2p/makefile
# if you just need miniperl, replace test_prep with miniperl
make test_prep
[ -x ./perl ] || exit 125
# This runs the actual testcase. You could use -e instead:
./perl -Ilib ~/testcase.pl
ret=$?
[ $ret -gt 127 ] && ret=127
git checkout makedepend.SH
git clean -dxf
exit $ret

#if you need to invert the exit code, replace the above exit with this:
#[ $ret -eq 0 ] && exit 1
#exit 0
