#!/bin/sh

# script_test_7.sh -- test for SEGMENT_START expressions.

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>.

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

# This file goes with script_test_4.t, which is a linker script which
# starts the program at an unaligned address.

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

check script_test_7.stdout "\\.interp[ 	]*PROGBITS[ 	]*0*10000100"
check script_test_7.stdout "\\.data[ 	]*PROGBITS[ 	]*0*10200000"
check script_test_7.stdout "\\.bss[ 	]*NOBITS[ 	]*0*1040...."
