#!/bin/sh

# gnu_property_test.sh -- test .note.gnu.property section.

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

# This script checks that after linking the three object files
# gnu_property_[abc].S, each of which contains a .note.gnu.property
# section, the resulting output has only a single such note section,
# and that the properties have been correctly combined.

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

check_count()
{
    if test "`grep -c "$2" "$1"`" != "$3"
    then
	echo "Did not find correct number of note sections in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_alignment ()
{
    if $EGREP -q "Class:[ \t]+ELF64" "$1"
    then
	align=8
    else
	align=4
    fi
    if ! $EGREP -q ".note.gnu.property[ \t]+NOTE.*$align$" "$1"
    then
	echo "Wrong .note.gnu.property alignment in $1:"
	$EGREP ".note.gnu.property[ \t]+NOTE.*$align" "$1"
	exit 1
    fi
}

check_alignment gnu_property_test.stdout

check_count gnu_property_test.stdout "GNU\s*0x[0-9a-f]*\s*NT_GNU_PROPERTY_TYPE_0" 1

check_count gnu_property_test.stdout "^  NOTE" 2

check gnu_property_test.stdout "stack size: 0x111100"
check gnu_property_test.stdout "no copy on protected"
check gnu_property_test.stdout "x86 ISA used: x86-64-baseline, <unknown: 10>, <unknown: 100>, <unknown: 1000>"
check gnu_property_test.stdout "x86 ISA needed: x86-64-baseline, <unknown: 10>, <unknown: 100>, <unknown: 1000>"
check gnu_property_test.stdout "x86 feature: IBT"

exit 0
