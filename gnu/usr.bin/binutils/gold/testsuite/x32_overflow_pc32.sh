#!/bin/sh

# x86_64_overflow_pc32.sh -- a test case for overflow checking.

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

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected error in $1:"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

# We don't know how the compiler might order these variables, so we
# can't test for the actual offset from .data, hence the regexp.
check x86_64_overflow_pc32.err "function bar:(.*): error: relocation overflow"

exit 0
