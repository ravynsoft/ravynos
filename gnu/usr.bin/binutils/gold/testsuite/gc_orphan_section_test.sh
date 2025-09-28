#!/bin/sh

# gc_orphan_section_test.sh -- test --gc-sections

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Sriraman Tallam <tmsriram@google.com>.

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

# The goal of this program is to verify if gc-sections works as expected
# with orphan sections.
# File gc_orphan_sections_test.cc is in this test. This program checks if
# the orphan sections are retained when they are referenced through
# __start_XXX and __stop_XXX symbols.

check()
{
    if grep -q " boo" "$1"
    then
        echo "Garbage collection failed to collect boo"
	exit 1
    fi
    grep_foo=`grep -q " foo" $1`
    if [ $? != 0 ];
    then
        echo "Garbage collection should not discard foo"
	exit 1
    fi
}

check gc_orphan_section_test.stdout
