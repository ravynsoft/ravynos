#!/bin/sh

# plugin_test_7.sh -- a test case for the plugin API with GC.

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
	echo "Did not find expected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}

check_not()
{
    if grep -q "$2" "$1"
    then
	echo "Found unexpected output in $1:"
	echo "   $2"
	echo ""
	echo "Actual output below:"
	cat "$1"
	exit 1
    fi
}


check plugin_test_7.err "set_x: PREVAILING_DEF_IRONLY"
check plugin_test_7.err "fun2: RESOLVED_EXEC"
check plugin_test_7.err "fun1: PREVAILING_DEF_REG"
check plugin_test_7.err "removing unused section from '.text.fun2' in file 'plugin_test_7_2.o'"
check_not plugin_test_7.syms "fun2"
