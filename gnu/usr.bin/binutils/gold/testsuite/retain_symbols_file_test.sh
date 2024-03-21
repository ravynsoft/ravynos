#!/bin/sh

# retain_symbols_file_test.sh -- a test case for -retain-symbols-file

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Craig Silverstein <csilvers@google.com>.

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

# The Makefile tries linking simple_test.o with -retain-symbols-file.
# It then runs nm over the results.  We check that the output is right.

check_present()
{
    if ! grep -q "$1" retain_symbols_file_test.stdout
    then
        echo "Did not find expected symbol $1 in retain_symbols_file_test.stdout"
        exit 1
    fi
}

check_absent()
{
    if grep -q "$1" retain_symbols_file_test.stdout
    then
        echo "Found unexpected symbol $1 in retain_symbols_file_test.stdout"
        exit 1
    fi
}

check_present 't1'
check_present 't20a::get()'
check_present 't18()'
check_absent 't10'
check_absent 't1()'
check_absent 't16b::t()'

exit 0
