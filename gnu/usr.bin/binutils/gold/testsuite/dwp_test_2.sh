#!/bin/sh

# dwp_test_2.sh -- Test the dwp tool.

# Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected output:"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

check_num()
{
    n=$(grep -c "$2" "$1")
    if test "$n" -ne "$3"
    then
	echo "Found $n occurrences (should find $3):"
	echo "   $2"
	echo ""
	echo "Actual error output below:"
	cat "$1"
	exit 1
    fi
}

STDOUT="dwp_test_2.stdout"

check $STDOUT "^Contents of the .debug_info.dwo section"
check_num $STDOUT "DW_TAG_compile_unit" 4
check_num $STDOUT "DW_TAG_type_unit" 3
check_num $STDOUT "DW_AT_name.*: C1" 3
check_num $STDOUT "DW_AT_name.*: C2" 2
check_num $STDOUT "DW_AT_name.*: C3" 3
check_num $STDOUT "DW_AT_name.*: testcase1" 6
check_num $STDOUT "DW_AT_name.*: testcase2" 6
check_num $STDOUT "DW_AT_name.*: testcase3" 6
check_num $STDOUT "DW_AT_name.*: testcase4" 4
