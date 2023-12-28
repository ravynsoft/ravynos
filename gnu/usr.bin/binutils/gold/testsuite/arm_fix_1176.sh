#!/bin/sh

# arm_fix_1176.sh -- a test case for the ARM1176 workaround.

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

# Check for fix default state on v6Z.
check arm_fix_1176_default_v6z.stdout "2001014:	.*	bl	2001018 <.*>"

# Check for fix explicitly on on v6Z.
check arm_fix_1176_on_v6z.stdout "2001014:	.*	bl	2001018 <.*>"

# Check for explicitly off on v6Z
check arm_fix_1176_off_v6z.stdout "2001014:	.*	blx	2001018 <.*>"

# Check for fix default state on v5TE
check arm_fix_1176_default_v5te.stdout "2001014:	.*	bl	2001018 <.*>"

# Check for fix default state on v7A
check arm_fix_1176_default_v7a.stdout "2001014:	.*	blx	2001018 <.*>"

# Check for fix default state on ARM1156T2F-S
check arm_fix_1176_default_1156t2f_s.stdout "2001014:	.*	blx	2001018 <.*>"

exit 0
