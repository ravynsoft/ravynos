#!/bin/sh

# undef_symbol.sh -- a test case for undefined symbols in shared libraries

# Copyright (C) 2007-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

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

# This file goes with debug_msg.cc, a C++ source file constructed to
# have undefined references.  We compile that file with debug
# information and then try to link it, and make sure the proper errors
# are displayed.  The errors will be found in debug_msg.err.

check()
{
    if ! grep -q "$1" undef_symbol.err
    then
	echo "Did not find expected error:"
	echo "   $1"
	echo ""
	echo "Actual error output below:"
	cat undef_symbol.err
	exit 1
    fi
}

check "undef_symbol.so: error: undefined reference to 'a'"

exit 0
