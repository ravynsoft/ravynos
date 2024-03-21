#!/bin/sh

# script_test_14.sh -- test SORT_BY_INIT_PRIORITY

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

file="script_test_14.stdout"

check()
{
    section=$1
    pattern=$2
    found=`fgrep "Contents of section $section:" -A1 $file | tail -n 1`
    if test -z "$found"; then
        echo "Section \"$section\" not found in file $file"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    match_pattern=`echo "$found" | grep -e "$pattern"`
    if test -z "$match_pattern"; then
        echo "Expected pattern was not found in section \"$section\":"
        echo "    $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
}

# Sort order for .init_array:
# * .init_array      -- Doesn't have a numeric part, compared with others as strings.
# * .init_array.101  -- The numeric part is less than in the two others.
# * .init_array.0103 -- These names have numeric parts with the same value,
# * .init_array.103  /  so they are compared as strings.
check ".init_array" "\<00010304\b"

# Sort order for .fini_array, the same consideration as for .init_array:
# * .fini_array
# * .fini_array.101
# * .fini_array.0103
# * .fini_array.103
check ".fini_array" "\<f0f1f3f4\b"

# Sort order for .ctors:
# * .ctors      -- Doesn't have a numeric part, compared with others as strings
# * .ctors.0103 -- The numeric parts have the same value, which is greater than
# * .ctors.103  /  in the last section's name. This pair is compared as strings.
# * .ctors.101  -- The least numeric part among all sections which contain them.
check ".ctors" "\<c0c3c4c1\b"

# Sort order for .dtors, the same considerations as for .ctors:
# * .dtors
# * .dtors.0103
# * .dtors.103
# * .dtors.101
check ".dtors" "\<d0d3d4d1\b"

# Sort order for .sec, just sort as strings, because it's not the reserved name:
# * .sec
# * .sec.0103
# * .sec.101
# * .sec.103
check ".sec" "\<a0a3a1a4\b"

