#!/bin/sh

# pr14265.sh -- test --gc-sections with KEEP

# Copyright (C) 2012-2023 Free Software Foundation, Inc.
# Written by Nick Clifton  <nickc@redhat.com>

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
        echo "Garbage collection failed to KEEP :"
        echo "   $2"
	exit 1
    fi
}

check pr14265.stdout "foo1_start"
check pr14265.stdout "foo1_end"
check pr14265.stdout "foo2_start"
check pr14265.stdout "foo2_end"

