#!/bin/sh

# dynamic_list.sh -- test --dynamic-list and --dynamic-list-*

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

# This file goes with dynamic_list.t, which is a dynamic-list script.

check()
{
    if ! grep -qw "$2" "$1"
    then
	echo "Did not find expected text in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check dynamic_list.stdout "main"            # comes via --dynamic-list
check dynamic_list.stdout "_Z4t1_6v"        # t1_6()
check dynamic_list.stdout "_ZN4t16aD1Ev"    # t16a:~t16a()
check dynamic_list.stdout "_ZN4t16a1tEv"    # t16a:t()
check dynamic_list.stdout "_ZTI4t16a"       # typeinfo for t16a
check dynamic_list.stdout "_ZTI4t16b"       # typeinfo for t16b
check dynamic_list.stdout "_ZTS4t16a"       # typeinfo name for t16a
check dynamic_list.stdout "_ZTS4t16b"       # typeinfo name for t16b
check dynamic_list.stdout "t20v"            # comes via --dynamic-list-data
