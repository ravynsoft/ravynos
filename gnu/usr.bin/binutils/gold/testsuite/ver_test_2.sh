#!/bin/sh

# ver_test_2.sh -- test that symbol has correct version

# Copyright (C) 2008-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

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

# This file goes with ver_matching_def.cc, a C++ source file
# constructed with several symbols mapped via version_script.map.  We
# run readelf on the resulting shared object and check that each
# symbol has the correct version.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected symbol in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check ver_test_2.syms "t1_2@VER2"

exit 0
