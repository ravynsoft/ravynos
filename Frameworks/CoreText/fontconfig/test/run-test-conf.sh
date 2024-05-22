#!/bin/sh
# test/run-test-conf.sh
#
# Copyright © 2000 Keith Packard
# Copyright © 2018 Akira TAGOH
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author(s) not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  The authors make no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
set -e

case "$OSTYPE" in
    msys ) MyPWD=`pwd -W` ;;  # On Msys/MinGW, returns a MS Windows style path.
    *    ) MyPWD=`pwd`    ;;  # On any other platforms, returns a Unix style path.
esac

TESTDIR=${srcdir-"$MyPWD"}
BUILDTESTDIR=${builddir-"$MyPWD"}

RUNNER=../test/test-conf$EXEEXT

if [ ! -f ${RUNNER} ]; then
    echo "${RUNNER} not found!\n"
    echo "Building this test requires libjson-c development files to be available."
    exit 77 # SKIP
fi

for i in \
	45-generic.conf \
	60-generic.conf \
	90-synthetic.conf \
    ; do
    test_json=$(echo test-$i|sed s'/\.conf/.json/')
    echo $RUNNER $TESTDIR/../conf.d/$i $TESTDIR/$test_json
    $RUNNER $TESTDIR/../conf.d/$i $TESTDIR/$test_json
done
for i in \
	test-issue-286.json \
	test-style-match.json \
    ; do
    echo $RUNNER $TESTDIR/$i ...
    $RUNNER $TESTDIR/../conf.d/10-autohint.conf $TESTDIR/$i
done
