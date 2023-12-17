#!/bin/sh
# Unit tests for init.sh
# Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

: "${srcdir=.}"
. "$srcdir/init.sh"; path_prepend_ .

fail=0

test_compare()
{
  touch empty || fail=1
  echo xyz > in || fail=1

  compare /dev/null /dev/null >out 2>err || fail=1
  test -s out && fail_ "out not empty: $(cat out)"
  # "err" should be empty, too, but has "set -x" output when VERBOSE=yes
  case $- in *x*) ;; *) test -s err && fail_ "err not empty: $(cat err)";; esac

  compare /dev/null empty >out 2>err || fail=1
  test -s out && fail_ "out not empty: $(cat out)"
  case $- in *x*) ;; *) test -s err && fail_ "err not empty: $(cat err)";; esac

  compare in in >out 2>err || fail=1
  test -s out && fail_ "out not empty: $(cat out)"
  case $- in *x*) ;; *) test -s err && fail_ "err not empty: $(cat err)";; esac

  compare /dev/null in >out 2>err && fail=1
  cat <<\EOF > exp
diff -u /dev/null in
--- /dev/null	1970-01-01
+++ in	1970-01-01
+xyz
EOF
  compare exp out || fail=1
  case $- in *x*) ;; *) test -s err && fail_ "err not empty: $(cat err)";; esac

  compare empty in >out 2>err && fail=1
  # Compare against expected output only if compare is using diff -u.
  if grep @ out >/dev/null; then
    # Remove the TAB-date suffix on each --- and +++ line,
    # for both the expected and the actual output files.
    # Also remove the @@ line, since Solaris 5.10 and GNU diff formats differ:
    # -@@ -0,0 +1 @@
    # +@@ -1,0 +1,1 @@
    # Also, remove space after leading '+', since AIX 7.1 diff outputs a space.
    sed 's/	.*//;/^@@/d;s/^+ /+/' out > k && mv k out
    cat <<\EOF > exp
--- empty
+++ in
+xyz
EOF
    compare exp out || fail=1
  fi
  case $- in *x*) ;; *) test -s err && fail_ "err not empty: $(cat err)";; esac
}

test_compare

Exit $fail
