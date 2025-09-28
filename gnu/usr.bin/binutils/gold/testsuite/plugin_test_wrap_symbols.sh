#!/bin/sh

# plugin_test_wrap_symbols.sh -- a test case for the plugin API.

# Copyright (C) 2017-2023 Free Software Foundation, Inc.
# Written by Teresa Johnson <tejohnson@google.com>.

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

# This file goes with plugin_test.c, a simple plug-in library that
# exercises the basic interfaces and prints out version numbers and
# options passed to the plugin.

# This checks if the get_wrap_symbols plugin interface works as
# expected.

check()
{
    if ! grep -q "$2" "$1"
    then
	echo "Did not find expected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check plugin_test_wrap_symbols.err "API version:"
check plugin_test_wrap_symbols.err "gold version:"
check plugin_test_wrap_symbols.err "Number of wrap symbols = 2"
check plugin_test_wrap_symbols.err "Wrap symbol hello"
check plugin_test_wrap_symbols.err "Wrap symbol jello"
check plugin_test_wrap_symbols.err "cleanup hook called"

exit 0
