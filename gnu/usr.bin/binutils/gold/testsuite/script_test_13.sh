#!/bin/sh

# script_test_13.sh -- test that internally created sections obey
# the order from the linker script.

# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Written by Igor Kudrin <ikudrin@accesssoftek.com>.

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
    file=$1
    pattern=$2
    match_pattern=`grep -e "$pattern" $file`
    if test -z "$match_pattern"; then
        echo "Expected pattern was not found:"
        echo "    $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
}

check "script_test_13.stdout" "\.rela\?\.dyn[[:space:]]\+RELA\?[[:space:]]\+0\+10000\b"
