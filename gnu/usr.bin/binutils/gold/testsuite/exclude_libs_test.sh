#!/bin/sh

# exclude_libs_test.sh -- test that library symbols are not exported.

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

check()
{
    file=$1
    sym=$2
    vis=$3

    found=`grep " $sym\$" $file`
    if test -z "$found"; then
	echo "Symbol $sym not found."
	exit 1
    fi

    match_vis=`grep " $sym\$" $file | grep " $vis "`
    if test -z "$match_vis"; then
	echo "Expected symbol $sym to have visibility $vis but found"
	echo "$found"
	exit 1
    fi
}

check "exclude_libs_test.syms" "lib1_default" "HIDDEN"
check "exclude_libs_test.syms" "lib1_protected" "HIDDEN"
check "exclude_libs_test.syms" "lib1_internal" "INTERNAL"
check "exclude_libs_test.syms" "lib1_hidden" "HIDDEN"
check "exclude_libs_test.syms" "lib2_default" "DEFAULT"
check "exclude_libs_test.syms" "lib2_protected" "PROTECTED"
check "exclude_libs_test.syms" "lib2_internal" "INTERNAL"
check "exclude_libs_test.syms" "lib2_hidden" "HIDDEN"
check "exclude_libs_test.syms" "lib3_default" "HIDDEN"
check "exclude_libs_test.syms" "lib3_protected" "HIDDEN"
check "exclude_libs_test.syms" "lib3_internal" "INTERNAL"
check "exclude_libs_test.syms" "lib3_hidden" "HIDDEN"

exit 0
