#!/bin/sh

# arm_v4bx.sh -- a test case for --fix-v4bx and --fix-v4bx-interworking.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# This file goes with arm_v4bx.s, an ARM assembly source file constructed to
# have test the handling of R_ARM_V4BX relocation.

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

# Test --fix-v4bx
check arm_fix_v4bx.stdout ".*00:	.* 	mov	pc, r0"
check arm_fix_v4bx.stdout ".*04:	.* 	mov	pc, pc"

# Test --fix-v4bx-interworking
check arm_fix_v4bx_interworking.stdout ".*00:	.* 	b	.*00 <.*>"
check arm_fix_v4bx_interworking.stdout ".*04:	.* 	mov	pc, pc"
check arm_fix_v4bx_interworking.stdout ".*00:	.* 	tst	r0, #1"
check arm_fix_v4bx_interworking.stdout ".*04:	.* 	moveq	pc, r0"
check arm_fix_v4bx_interworking.stdout ".*08:	.* 	bx	r0"

# Test no fix.
check arm_no_fix_v4bx.stdout ".*00:	.* 	bx	r0"
check arm_no_fix_v4bx.stdout ".*04:	.* 	bx	pc"

exit 0
