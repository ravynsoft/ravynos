#!/bin/sh

# discard_locals_test.sh -- test that local symbols are discarded.

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>

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

# This file goes with exclude_libs_test.c, a C source file
# linked with option -Wl,--exclude-libs. We run readelf on
# the resulting executable and check that symbols from two test library
# archives are correctly hidden or left unmodified.

check_discarded()
{
    file=$1
    sym=$2

    found=`$EGREP $sym $file`
    if test -n "$found"; then
	echo "These local symbols are not discarded in $file:"
	echo "$found"
	exit 1
    fi
}

check_non_discarded()
{
    file=$1
    sym=$2

    found=`$EGREP $sym $file`
    if test -z "$found"; then
	echo "This local symbol is discarded in $file:"
	echo "$2"
	exit 1
    fi
}

check_discarded     "discard_locals_test.syms" "should_be_discarded"

check_non_discarded "discard_locals_relocatable_test1.syms" ".LC0"
check_discarded     "discard_locals_relocatable_test1.syms" "should_be_discarded"
check_non_discarded "discard_locals_relocatable_test2.syms" ".LC0"
check_discarded     "discard_locals_relocatable_test2.syms" "should_be_discarded"

exit 0
