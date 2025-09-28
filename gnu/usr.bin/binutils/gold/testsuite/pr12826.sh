#!/bin/sh

# pr12826.sh -- a test case for combining ARM arch attributes.

# Copyright (C) 2011-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>.

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

# This file goes with pr12826_1.s and pr12826_2.s, two ARM assembly source
# files constructed to test handling of arch attributes.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find attribute in $1:"
	echo "   $2"
	echo ""
	echo "Actual attribute below:"
	cat "$1"
	exit 1
    fi
}

# Check that arch is armv7e-m.
check pr12826.stdout "Tag_CPU_arch: v7E-M"

exit 0
