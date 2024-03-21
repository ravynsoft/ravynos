#!/bin/sh

# aarch64_tlsdesc.sh -- test R_AARCH64_TLSDESC_* relocations.

# Copyright (C) 2017-2023 Free Software Foundation, Inc.
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

file=aarch64_tlsdesc.stdout

get_address_by_reloc()
{
    var=$1
    pattern="\<R_AARCH64_TLSDESC\>"
    found=$(grep "$pattern" "$file")
    if test -z "$found"; then
        echo "GOT entry for a TLS symbol is not found in file $file."
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    eval $var="0x0$(echo $found | awk -F'[: ]' '{ print $1 }')"
}

check_adrp()
{
    pattern="\<adrp[[:space:]]\+x0, 0\>"
    found=$(grep "$pattern" "$file")
    if test -z "$found"; then
        echo "An ADRP immediate is supposed to be 0"
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
}

get_address_from_ldr()
{
    var=$1
    pattern="\<ldr[[:space:]]\+x1\>"
    found=$(grep "$pattern" "$file")
    if test -z "$found"; then
        echo "An LDR instruction is not found in file $file."
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    eval $var="$(echo $found | awk -F'[#\\]]' '{ print $2 }')"
}

get_address_from_add()
{
    var=$1
    pattern="\<add[[:space:]]\+x0\>"
    found=$(grep "$pattern" "$file")
    if test -z "$found"; then
        echo "An ADD instruction is not found in file $file."
        echo "Search pattern: $pattern"
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
    eval $var="$(echo $found | awk -F'#' '{ print $2 }')"
}

check_adrp
get_address_by_reloc address_by_reloc
get_address_from_ldr address_from_ldr
get_address_from_add address_from_add

if test $(($address_by_reloc)) -ne $(($address_from_ldr)); then
    echo "The address in LDR instruction is wrong."
    echo ""
    echo "Actual output below:"
    cat "$file"
    exit 1
fi

if test $(($address_by_reloc)) -ne $(($address_from_add)); then
    echo "The address in ADD instruction is wrong."
    echo ""
    echo "Actual output below:"
    cat "$file"
    exit 1
fi

exit 0
