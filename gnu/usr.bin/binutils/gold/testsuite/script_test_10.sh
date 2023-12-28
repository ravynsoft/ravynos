#!/bin/sh

# script_test_10.sh -- test for the section order.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Viktor Kutuzov <vkutuzov@accesssoftek.com>.

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
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected section in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check script_test_10.stdout ".*\[ 1\] .text"
check script_test_10.stdout ".*\[ 2\] .sec0"
check script_test_10.stdout ".*\[ 3\] .sec1"
check script_test_10.stdout ".*\[ 4\] .sec2"
check script_test_10.stdout ".*\[ 5\] .secz"
check script_test_10.stdout ".*\[ 6\] .sec3"
check script_test_10.stdout ".*\[ 7\] .data"
check script_test_10.stdout ".* .bss"

