#!/bin/sh

# script_test_5.sh -- test linker script with uncovered sections

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

# This file goes with script_test_5.t, which is a linker script with
# a SECTIONS clause that does not explicitly mention one of the input
# sections in the test object file.  We check to make sure that the
# correct output section is generated.

check_count()
{
    if test "`grep -c "$2" "$1"`" != "$3"
    then
	echo "Did not find expected number ($3) of '$2' sections in $1"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_count script_test_5.stdout " .text " 1
check_count script_test_5.stdout " .text.foo " 1
