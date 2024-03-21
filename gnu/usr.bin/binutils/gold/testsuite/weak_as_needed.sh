#!/bin/sh

# weak_as_needed.sh -- a test case for version handling with weak symbols
# and --as-needed libraries.

# Copyright (C) 2018-2023 Free Software Foundation, Inc.
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

# This test verifies that a weak reference is properly bound to
# an as-needed library, when it is first resolved to a symbol in
# a library that ends up being not needed.

# Ref: https://stackoverflow.com/questions/50751421/undefined-behavior-in-shared-lib-using-libpthread-but-not-having-it-in-elf-as-d

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_missing()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check weak_as_needed.stdout "WEAK .* UND  *bar@v2"
check weak_as_needed.stdout "NEEDED.*weak_as_needed_c\.so"
check_missing weak_as_needed.stdout "NEEDED.*weak_as_needed_b\.so"

exit 0
