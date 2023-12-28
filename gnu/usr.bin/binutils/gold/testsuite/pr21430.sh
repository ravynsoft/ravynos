#!/bin/sh

# pr21430.sh -- test the position of a relaxed section.

# Copyright (C) 2017-2023 Free Software Foundation, Inc.
# Written by Igor Kudrin  <ikudrin@accesssoftek.com>

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

file=pr21430.stdout

get_symbol_address()
{
    symbol=$1
    var=$2
    pattern="\<$symbol\>"
    found=`grep "$pattern" "$file"`
    if test -z "$found"; then
        echo "Symbol '$symbol' not found in file $file."
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    eval $var="0x0`echo $found | awk '{ print $1 }'`"
}

get_symbol_size()
{
    symbol=$1
    var=$2
    pattern="\<$symbol\>"
    found=`grep "$pattern" "$file"`
    if test -z "$found"; then
        echo "Symbol '$symbol' not found in file $file."
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    eval $var="0x0`echo $found | awk '{ print $2 }'`"
}

get_symbol_address "bar" bar_address
get_symbol_size "bar" bar_size
get_symbol_address "foo" foo_address

if test $(($foo_address)) -lt $(($bar_address+$bar_size)); then
    echo "'foo' should not overlap the content of 'bar'."
    echo ""
    echo "Actual output below:"
    cat "$file"
    exit 1
fi

exit 0
