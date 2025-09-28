#!/bin/sh

# ver_test_8.sh -- check that __GLOBAL_OFFSET_TABLE__ is defined only once.

# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@gmail.com>.

# This file is part of gold.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

count=`grep -c -E '(_GLOBAL_OFFSET_TABLE_|\.TOC\.)' ver_test_8_2.so.syms`

if test "$count" -gt 1; then
  echo "Found $count copies of '_GLOBAL_OFFSET_TABLE_|.TOC.' (should be only 1)"
  exit 1
fi

exit 0
