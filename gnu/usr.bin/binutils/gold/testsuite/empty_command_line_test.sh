#!/bin/sh

# empty_command_line_test.sh -- test various command lines with no inputs

# Copyright (C) 2017-2023 Free Software Foundation, Inc.
# Written by Benjamin Peterson <bp@benjamin.pe>

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
    expected_msg="$1"
    shift
    echo "checking empty command line '$@'"
    err=$(gcctestdir/ld "$@" 2>&1)
    if [ $? != 1 ]; then
        echo "gold didn't fail"
        exit 1
    fi
    echo "$err" | grep -q "$expected_msg" || {
        echo "unexpected error message: $err"
        exit 1
    }
}

check "no input files"
check "missing lib end" --start-lib
check "missing group end" --start-group
