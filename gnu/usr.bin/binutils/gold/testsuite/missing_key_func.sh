#!/bin/sh

# missing_key_func.sh -- a test case for printing error messages when
# a class is missing its key function.

# Copyright (C) 2013-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@google.com>

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

# This file goes with debug_msg.cc, a C++ source file constructed to
# have undefined references.  We compile that file with debug
# information and then try to link it, and make sure the proper errors
# are displayed.  The errors will be found in debug_msg.err.

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

check_missing()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected error in $1:"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

check missing_key_func.err "error: undefined reference to 'vtable for C'"
check missing_key_func.err "class is missing its key function"
