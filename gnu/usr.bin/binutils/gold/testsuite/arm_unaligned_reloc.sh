#!/bin/sh

# arm_unaligned_reloc.sh -- test ARM unaligned static data relocations.

# Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

# This file goes with the assembler source file arm_unaligned_reloc.s,
# that is assembled and linked as a dummy executable.  We want to check
# it is okay to do unaligned static data relocations.

check()
{
    if ! grep -q -e "$2" "$1"
    then
	echo "Did not find pattern \"$2\" in $1:"
	echo "   $2"
	echo ""
	echo "Actual disassembly below:"
	cat "$1"
	exit 1
    fi
}

check arm_unaligned_reloc.stdout "^00009000 <x>:$"
check arm_unaligned_reloc.stdout "^0000a001 <abs32>:$"
check arm_unaligned_reloc.stdout '^[	 ]*a001:[	 ]*00009001[	 ].*$'
check arm_unaligned_reloc.stdout "^0000a005 <rel32>:"
check arm_unaligned_reloc.stdout "^[	 ]*a005:[	 ]*ffffeffc[	 ].*$"
check arm_unaligned_reloc.stdout "^0000a009 <abs16>:"
check arm_unaligned_reloc.stdout "^[	 ]*a009:[	 ]*00009001[	 ].*$"

check arm_unaligned_reloc_r.stdout "^[	 ]*1:[	 ]*00000001[	 ].*$"
check arm_unaligned_reloc_r.stdout "^[	]*1: R_ARM_ABS32[	]*.data.0$"
check arm_unaligned_reloc_r.stdout "^[	 ]*5:[	 ]*00000001[	 ].*$"
check arm_unaligned_reloc_r.stdout "^[	]*5: R_ARM_REL32[	]*.data.0$"
check arm_unaligned_reloc_r.stdout "^[	 ]*9:[	 ]*00000001[	 ].*$"
check arm_unaligned_reloc_r.stdout "^[	]*9: R_ARM_ABS16[	]*.data.0$"

exit 0
