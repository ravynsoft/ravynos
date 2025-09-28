#!/bin/sh

# no_version_test.sh -- test that .gnu.version* sections are not created
# in a shared object when symbol versioning is not used.

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>

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

# This file goes with no_version_test.c, a C source file
# linked with option -shared -nostdlib.  We run objdump on
# the resulting executable and check that .gnu.version* sections
# are not created.

check()
{
    file=$1

    found=`$EGREP "\.gnu\.version.*" $file`
    if test -n "$found"; then
	echo "These section should not be in $file:"
	echo "$found"
	exit 1
    fi
}

check "no_version_test.stdout"

exit 0
