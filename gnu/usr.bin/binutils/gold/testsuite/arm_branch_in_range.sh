#!/bin/sh

# arm_branch_in_range.sh -- test ARM/THUMB/THUMB branch instructions whose
# targets are just within the branch range limits.

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

# This file goes with the assembler source files arm_bl_in_range.s
# thumb_bl_in_range.s that are assembled and linked to check that branches
# whose target are just within the branch range limits are handle correctly.

check()
{
    file=$1
    pattern=$2

    found=`grep "$pattern" $file`
    if test -z "$found"; then
	echo "pattern \"$pattern\" not found in file $file."
	exit 1
    fi
}

# This is a bit crude.  Also, there are tabs in the grep patterns. 

check arm_bl_in_range.stdout \
  " 4000004:	eb800000 	bl	200000c <_backward_target>"
check arm_bl_in_range.stdout \
  " 4000008:	eb7fffff 	bl	600000c <_forward_target>"
check thumb_bl_in_range.stdout \
  " 800004:	f400 f800 	bl	400008 <_backward_target>"
check thumb_bl_in_range.stdout \
  " 800008:	f3ff ffff 	bl	c0000a <_forward_target>"
check thumb2_bl_in_range.stdout \
 " 2000004:	f400 d000 	bl	1000008 <_backward_target>"
check thumb2_bl_in_range.stdout \
 " 2000008:	f3ff d7ff 	bl	300000a <_forward_target>"
check thumb_blx_in_range.stdout \
 " 800006:	f400 e800 	blx	400008 <_backward_target>"
check thumb_blx_in_range.stdout \
 " 80000c:	f3ff effe 	blx	c0000c <_forward_target>"
check thumb2_blx_in_range.stdout \
 " 2000006:	f400 c000 	blx	1000008 <_backward_target>"
check thumb2_blx_in_range.stdout \
 " 200000c:	f3ff c7fe 	blx	300000c <_forward_target>"
check arm_thm_jump11.stdout \
 "    8804:	e400      	b.n	8008 <_backward_target>"
check arm_thm_jump11.stdout \
 "    8806:	e3ff      	b.n	9008 <_forward_target>"
check arm_thm_jump8.stdout \
 "    8104:	d080      	beq.n	8008 <_backward_target>"
check arm_thm_jump8.stdout \
 "    8106:	d07f      	beq.n	8208 <_forward_target>"

exit 0
