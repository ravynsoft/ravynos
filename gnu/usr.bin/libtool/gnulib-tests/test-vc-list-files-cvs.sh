#!/bin/sh
# Unit tests for vc-list-files
# Copyright (C) 2008-2022 Free Software Foundation, Inc.
# This file is part of the GNUlib Library.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

: ${srcdir=.}
. "$srcdir/init.sh"; path_prepend_ .

tmpdir=vc-cvs
repo=`pwd`/$tmpdir/repo

fail=0
for i in with-cvsu without; do
  # On the first iteration, test using cvsu, if it's in your path.
  # On the second iteration, ensure that cvsu fails, so we'll
  # exercise the awk-using code.
  if test $i = without; then
    printf '%s\n' '#!/bin/sh' 'exit 1' > cvsu
    chmod a+x cvsu
    PATH=`pwd`:$PATH
    export PATH
  fi
  ok=0
  mkdir $tmpdir && cd $tmpdir &&
    # without cvs, skip the test
    { ( cvs -Q -d "$repo" init ) > /dev/null 2>&1 \
      || skip_ "cvs not found in PATH"; } &&
    mkdir w && cd w &&
    mkdir d &&
    touch d/a b c &&
    cvs -Q -d "$repo" import -m imp m M M0 &&
    cvs -Q -d "$repo" co m && cd m &&
    printf '%s\n' b c d/a > expected &&
    $BOURNE_SHELL "$abs_aux_dir/vc-list-files" | sort > actual &&
    compare expected actual &&
    ok=1
  test $ok = 0 && fail=1
done

Exit $fail
