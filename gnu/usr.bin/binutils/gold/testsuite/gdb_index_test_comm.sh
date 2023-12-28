#!/bin/sh

# gdb_index_test_comm.sh -- common code for --gdb-index tests.

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

STDOUT="$1"

check $STDOUT "^Version [4-7]"

# Look for the symbols we know should be in the symbol table.

check $STDOUT "^\[ *[0-9]*\] (anonymous namespace):"
check $STDOUT "^\[ *[0-9]*\] (anonymous namespace)::c1_count:"
check $STDOUT "^\[ *[0-9]*\] (anonymous namespace)::c2_count:"
check $STDOUT "^\[ *[0-9]*\] bool:"
check $STDOUT "^\[ *[0-9]*\] check<one::c1>:"
check $STDOUT "^\[ *[0-9]*\] check<two::c2<double> >:"
check $STDOUT "^\[ *[0-9]*\] check<two::c2<int> >:"
# check $STDOUT "^\[ *[0-9]*\] check<two::c2<int const\*> >:"
check $STDOUT "^\[ *[0-9]*\] double:"
check $STDOUT "^\[ *[0-9]*\] F_A:"
check $STDOUT "^\[ *[0-9]*\] F_B:"
check $STDOUT "^\[ *[0-9]*\] F_C:"
check $STDOUT "^\[ *[0-9]*\] int:"
check $STDOUT "^\[ *[0-9]*\] main:"
check $STDOUT "^\[ *[0-9]*\] one:"
check $STDOUT "^\[ *[0-9]*\] one::c1:"
check $STDOUT "^\[ *[0-9]*\] one::c1::~c1:"
check $STDOUT "^\[ *[0-9]*\] one::c1::c1:"
check $STDOUT "^\[ *[0-9]*\] one::c1::val:"
check $STDOUT "^\[ *[0-9]*\] one::c1v:"
check $STDOUT "^\[ *[0-9]*\] one::G_A:"
check $STDOUT "^\[ *[0-9]*\] one::G_B:"
check $STDOUT "^\[ *[0-9]*\] one::G_B:"
check $STDOUT "^\[ *[0-9]*\] two:"
check $STDOUT "^\[ *[0-9]*\] two::c2<double>::~c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<double>::c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<double>::val:"
check $STDOUT "^\[ *[0-9]*\] two::c2<double>:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int const\*>:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int const\*>::~c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int const\*>::c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int const\*>::val:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int>::~c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int>::c2:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int>::val:"
check $STDOUT "^\[ *[0-9]*\] two::c2<int>:"
check $STDOUT "^\[ *[0-9]*\] two::c2v1:"
check $STDOUT "^\[ *[0-9]*\] two::c2v2:"
check $STDOUT "^\[ *[0-9]*\] anonymous_union_var:"
check $STDOUT "^\[ *[0-9]*\] inline_func_1:"

exit 0
