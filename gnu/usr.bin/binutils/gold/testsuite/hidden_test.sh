#!/bin/sh

# hidden_test.sh -- a test case for hidden and internal symbols.

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@google.com>.

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

# This file goes with hidden_test_main.c and hidden_test_1.c.
# The main program defines several symbols with each of the ELF
# visibilities, and the shared library attempts to reference the
# symbols.  We try to link the program and check that the expected
# error messages are issued for the references to internal and
# hidden symbols.  The errors will be found in hidden_test.err.

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

check_missing_sym()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual nm output below:"
	cat "$1"
	exit 1
    fi
}

# We shouldn't see errors for the default and protected symbols.
check_missing hidden_test.err "main_default"
check_missing hidden_test.err "main_protected"

# We shouldn't see errors for the hidden and internal symbols either (PR 15574).
check_missing hidden_test.err "main_hidden"
check_missing hidden_test.err "main_internal"

# We shouldn't see the hidden or internal symbols in the dynamic symbol table.
check_missing_sym hidden_test.syms "main_hidden"
check_missing_sym hidden_test.syms "main_internal"

exit 0
