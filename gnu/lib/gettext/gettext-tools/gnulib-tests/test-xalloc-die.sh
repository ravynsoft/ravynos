#!/bin/sh
# Test suite for xalloc_die.
# Copyright (C) 2009-2023 Free Software Foundation, Inc.
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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

. "${srcdir=.}/init.sh"; path_prepend_ .

${CHECKER} test-xalloc-die${EXEEXT} > out 2> err
case $? in
  1) ;;
  *) Exit 1;;
esac

tr -d '\015' < err \
  | sed 's,.*test-xalloc-die[.ex]*:,test-xalloc-die:,' > err2 || Exit 1

compare - err2 <<\EOF || Exit 1
test-xalloc-die: memory exhausted
EOF

test -s out && Exit 1

Exit $fail
