#!/bin/sh

# arm_branch_out_of_range.sh -- test ARM/THUMB/THUMB branch instructions whose
# targets are just out of the branch range limits.

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

# This file goes with the assembler source files arm_bl_out_of_range.s,
# thumb_bl_out_of_range.s and thumb_bl_out_of_range_local.s that are assembled
# and linked to check that branches whose target are just out of the branch
# range limits are handle correctly.

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

check arm_bl_out_of_range.stdout \
  " 4000004:	eb00003d 	bl	4000100 <.*>"
check arm_bl_out_of_range.stdout \
  " 4000008:	eb00003e 	bl	4000108 <.*>"
check arm_bl_out_of_range.stdout \
  " 4000100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check arm_bl_out_of_range.stdout \
  " 4000104:	02000008 "
check arm_bl_out_of_range.stdout \
  " 4000108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check arm_bl_out_of_range.stdout \
  " 400010c:	06000010 "

check thumb_bl_out_of_range.stdout \
  " 800004:	f000 e87c 	blx	800100 <.*>"
check thumb_bl_out_of_range.stdout \
  " 800008:	f000 e87e 	blx	800108 <.*>"
check thumb_bl_out_of_range.stdout \
  " 800100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_bl_out_of_range.stdout \
  " 800104:	00400007 "
check thumb_bl_out_of_range.stdout \
  " 800108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_bl_out_of_range.stdout \
  " 80010c:	00c0000d "

check thumb_blx_out_of_range.stdout \
  " 800004:	f000 e87c 	blx	800100 <.*>"
check thumb_blx_out_of_range.stdout \
  " 80000a:	f000 e87e 	blx	800108 <.*>"
check thumb_blx_out_of_range.stdout \
  " 800100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_blx_out_of_range.stdout \
  " 800104:	00400004 "
check thumb_blx_out_of_range.stdout \
  " 800108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_blx_out_of_range.stdout \
  " 80010c:	00c0000c "

check thumb_bl_out_of_range_local.stdout \
  " 800004:	f000 e87c 	blx	800100 <.*>"
check thumb_bl_out_of_range_local.stdout \
  " 800008:	f000 e87e 	blx	800108 <.*>"
check thumb_bl_out_of_range_local.stdout \
  " 800100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_bl_out_of_range_local.stdout \
  " 800104:	00400007 "
check thumb_bl_out_of_range_local.stdout \
  " 800108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb_bl_out_of_range_local.stdout \
  " 80010c:	00c0000d "

check thumb2_bl_out_of_range.stdout \
  " 2000004:	f000 e87c 	blx	2000100 <.*>"
check thumb2_bl_out_of_range.stdout \
  " 2000008:	f000 e87e 	blx	2000108 <.*>"
check thumb2_bl_out_of_range.stdout \
  " 2000100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb2_bl_out_of_range.stdout \
  " 2000104:	01000007 "
check thumb2_bl_out_of_range.stdout \
  " 2000108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb2_bl_out_of_range.stdout \
  " 200010c:	0300000d "

check thumb2_blx_out_of_range.stdout \
  " 2000004:	f000 e87c 	blx	2000100 <.*>"
check thumb2_blx_out_of_range.stdout \
  " 200000a:	f000 e87e 	blx	2000108 <.*>"
check thumb2_blx_out_of_range.stdout \
  " 2000100:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb2_blx_out_of_range.stdout \
  " 2000104:	01000004 "
check thumb2_blx_out_of_range.stdout \
  " 2000108:	e51ff004 	ldr	pc, \[pc, #-4\]"
check thumb2_blx_out_of_range.stdout \
  " 200010c:	0300000c "

exit 0
