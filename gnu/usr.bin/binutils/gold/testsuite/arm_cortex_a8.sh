#!/bin/sh

# arm_cortex_a8.sh -- a test case for the Cortex-A8 workaround.

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

# Test branch.
check arm_cortex_a8_b.stdout ".*ffe:	.* 	b.w	.*000 <.*>"
check arm_cortex_a8_b.stdout ".000:	.* 	b.w	.*100 <_func>"

# Test conditional branch.
check arm_cortex_a8_b_cond.stdout ".*ffe:	.* 	b.w	.*000 <.*>"
check arm_cortex_a8_b_cond.stdout ".000:	.* 	beq.n	.*006 <.*>"
check arm_cortex_a8_b_cond.stdout ".002:	.* 	b.w	.*002 <.*>"
check arm_cortex_a8_b_cond.stdout ".006:	.* 	b.w	.*100 <_func>"

# Test branch and link.
check arm_cortex_a8_bl.stdout ".*ffe:	.* 	bl	.*000 <.*>"
check arm_cortex_a8_bl.stdout ".000:	.* 	b.w	.*100 <_func>"

# Test blx
check arm_cortex_a8_blx.stdout ".*ffe:	.* 	blx	.*000 <.*>"
check arm_cortex_a8_blx.stdout ".000:	.* 	b	.*100 <_func>"

# Test a local branch without relocation.
check arm_cortex_a8_local.stdout ".*ffe:	.* 	b.w	.*000 <.*>"
check arm_cortex_a8_local.stdout ".000:	.* 	bpl.n	.*006 <.*>"
check arm_cortex_a8_local.stdout ".002:	.* 	b.w	.*002 <.*>"
check arm_cortex_a8_local.stdout ".006:	.* 	b.w	.*100 <.*>"

exit 0
