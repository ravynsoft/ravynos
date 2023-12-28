#!/bin/sh

# arm_abs_global.sh -- test ARM absolute relocations against global symbols.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# This file goes with the assembler source file arm_abs_global.s,
# that is assembled and linked with a shared object libarm_abs.so.  We
# want to check that a MOV[TW] instruction referencing an external function
# causes a PLT to be created.

check()
{
    file=$1
    sym=$2
    reloc=$3

    found=`grep " $sym\$" $file`
    if test -z "$found"; then
	echo "Symbol $sym not found."
	exit 1
    fi

    match_reloc=`grep " $sym\$" $file | grep " $reloc "`
    if test -z "$match_reloc"; then
	echo "Expected symbol $sym to have relocation $reloc but found"
	echo "$found"
	exit 1
    fi
}

check "arm_abs_global.stdout" "_movt_abs_global" "R_ARM_JUMP_SLOT"
check "arm_abs_global.stdout" "_movw_abs_global" "R_ARM_JUMP_SLOT"
check "arm_abs_global.stdout" "_thm_movt_abs_global" "R_ARM_JUMP_SLOT"
check "arm_abs_global.stdout" "_thm_movw_abs_global" "R_ARM_JUMP_SLOT"
check "arm_abs_global.stdout" "_abs32_global_plt" "R_ARM_JUMP_SLOT"
check "arm_abs_global.stdout" "_abs32_global" "R_ARM_ABS32"

exit 0
