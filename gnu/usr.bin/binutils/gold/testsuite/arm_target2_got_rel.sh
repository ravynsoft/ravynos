#!/bin/sh

# arm_target2_got_rel.sh -- test --target2=got-rel options.
# This test is based on ld/testsuite/ld-arm/arm-target2-got-rel.d.

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
    section=$2
    pattern=$3
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

# .got=0x9000
# .got - 0x8000 = 0x1000
# .got - 0x8004 + 0x1234 = 0x2230
# .got - 0x8008 + 0xcdef0000 = 0xcdef0ff8
# .got - 0x800c + 0x76543210 = 0x76544204
check "arm_target2_got_rel.stdout" ".text" "\<8000[[:space:]]\+\(00100000\|00001000\)[[:space:]]\+\(30220000\|00002230\)[[:space:]]\+\(f80fefcd\|cdef0ff8\)[[:space:]]\+\(04425476\|76544204\)\b"

# foo = 0x8010
check "arm_target2_got_rel.stdout" ".got" "\<9000[[:space:]]\+\(10800000\|00008010\)\b"

exit 0
