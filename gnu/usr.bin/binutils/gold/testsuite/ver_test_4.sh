#!/bin/sh

# ver_test_4.sh -- test that version symbol is visible.

# Copyright (C) 2008-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

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

check()
{
    if ! sed '/\.symtab/q' "$1" | grep -q "$2"
    then
	echo "Did not find expected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	sed '/\.symtab/q' "$1"
	exit 1
    fi
}

check ver_test_4.syms "t1_2\$"
check ver_test_4.syms "t1_2@@VER2"
check ver_test_4.syms "t2_2@VER1"
check ver_test_4.syms "t2_2@@VER2"
check ver_test_4.syms "GLOBAL.*ABS.*VER1"
check ver_test_4.syms "GLOBAL.*ABS.*VER2"

exit 0
