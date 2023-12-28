#!/bin/sh

# arm_farcall_thumb_arm.sh -- a test case for Thumb->ARM farcall veneers.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Matthew Gretton-Dann <matthew.gretton-dann@arm.com>
# Based upon arm_cortex_a8.sh
# Written by Doug Kwan <dougkwan@google.com>.

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
	echo "Did not find expected instruction in $1:"
	echo "   $2"
	echo ""
	echo "Actual instructions below:"
	cat "$1"
	exit 1
    fi
}

# Thumb->ARM
check arm_farcall_thumb_arm.stdout "1f01018:	.*      	bx	pc"
check arm_farcall_thumb_arm.stdout "1f0101a:	.*      	nop"
check arm_farcall_thumb_arm.stdout "1f0101c:	f004 e51f"
check arm_farcall_thumb_arm.stdout "1f01020:	1014"
check arm_farcall_thumb_arm.stdout "1f01022:	0200"

check arm_farcall_thumb_arm.stdout "1f01024:	.*      	bx	pc"
check arm_farcall_thumb_arm.stdout "1f01026:	.*      	nop"
check arm_farcall_thumb_arm.stdout "1f01028:	fff9 ea03"

# Thumb->ARM with v5T interworking
check arm_farcall_thumb_arm_5t.stdout "1f01018:	f004 e51f"
check arm_farcall_thumb_arm_5t.stdout "1f0101c:	1014"
check arm_farcall_thumb_arm_5t.stdout "1f0101e:	0200"

exit 0
