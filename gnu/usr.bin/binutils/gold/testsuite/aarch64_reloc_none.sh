#!/bin/sh

# aarch64_reloc_none.sh -- test that R_AARCH64_NONE can be used
# to prevent garbage collecting a section.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# The goal of this test is to verify that support for the R_AARCH64_NONE
# relocation is implemented and it can be used to inform a linker that
# a section should be preserved during garbage collecting.
# File aarch64_reloc_none.s describes two similar sections, .foo and .bar,
# but .foo is referenced from .text using R_AARCH64_NONE against symbol foo.
# When flag --gc-sections is used, .foo and its symbol foo have to be
# preserved, whereas .bar and its symbol bar have to be discarded.

check()
{
    file=$1

    found_bar=`grep -e "\<bar\b" "$file"`
    if test -n "$found_bar"; then
	echo "Garbage collection failed to collect bar"
	echo ""
	echo "Actual output below:"
	cat "$file"
	exit 1
    fi

    found_foo=`grep -e "\<foo\b" "$file"`
    if test -z "$found_foo"; then
	echo "Garbage collection should not discard foo"
	echo ""
	echo "Actual output below:"
	cat "$file"
	exit 1
    fi
}

check aarch64_reloc_none.stdout
