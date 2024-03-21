#!/bin/sh

# ver_test_10.sh -- test global/local symbols

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

# This file goes with ver_test_4.script and ver_test_5.script.  The
# symbol t2_2 is not defined when ver_test_5.script is used.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check ver_test_10.syms "GLOBAL.*t3_2"
check ver_test_10.syms "LOCAL.*t4_2"

exit 0
