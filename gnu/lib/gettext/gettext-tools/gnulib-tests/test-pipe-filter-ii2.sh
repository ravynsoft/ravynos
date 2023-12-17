#! /bin/sh

# pipe-filter test driver.
#
# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Paolo Bonzini <bonzini@gnu.org>, 2009.
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

${CHECKER} ./test-pipe-filter-ii2-main${EXEEXT} ./test-pipe-filter-ii2-child${EXEEXT} | {
  set -e
  read a && test "$a" = "Test 1 passed."
  read a && test "$a" = "Test 2 passed."
  i=1
  while test $i -le 100; do
    read a && test "$a" = "abcdefghijklmnopqrstuvwxyz$i"
    i=`expr $i + 1`
  done
  read a && test "$a" = "Test 3 passed."
}
