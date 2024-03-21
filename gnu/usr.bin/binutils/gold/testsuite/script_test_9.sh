#!/bin/sh

# script_test_9.sh -- Check that the script_test_9.t script has placed
# .init and .text in the same segment.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Rafael Avila de Espindola <espindola@google.com>.

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
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected section in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check script_test_9.stdout "LOAD .*R E "
check script_test_9.stdout "LOAD .*RW "
check script_test_9.stdout "00 .*\.text"
check script_test_9.stdout "00 .*\.init"
check script_test_9.stdout "01 .*\.data "
