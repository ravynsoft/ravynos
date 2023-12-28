#!/bin/sh

# arm_farcall_arm_thumb.sh -- a test case for ARM->Thumb farcall veneers.

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

# Check for ARM->Thumb default
check arm_farcall_arm_thumb.stdout "1004:	.* 	ldr	ip, \[pc\]"
check arm_farcall_arm_thumb.stdout "1008:	.* 	bx	ip"
check arm_farcall_arm_thumb.stdout "100c:	02001015"

# Check for ARM->Thumb with v5t interworking
chck arm_farcall_arm_thumb_5t.stdout "1004:	f004 e51f"
chck arm_farcall_arm_thumb_5t.stdout "1008:	1015"
chck arm_farcall_arm_thumb_5t.stdout "100a:	0200"

exit 0
