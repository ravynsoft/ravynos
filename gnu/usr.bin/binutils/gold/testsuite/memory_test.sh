#!/bin/sh

# memory_test.sh -- test MEMORY regions.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
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


# NOTE: The linker script used in this test (memory_test.t)
# should be the same as the one used in the rgn-at5 linker
# test (ld/testsuite/ld-scripts/rgn-at5.t).
#
# Modulo some section ordering the output from GOLD in this
# test should be the same as the output from GNU LD in the
# rgn-at5 test.

check()
{
    file=$1
    pattern=$2
    found=`grep "$pattern" $file`
    if test -z "$found"; then
        echo "pattern \"$pattern\" not found in file $file."
	echo $found
        exit 1
    fi
}

check memory_test.stdout \
  "  LOAD           0x001000 0x0*02000 0x0*02000 0x0*04 0x0*04 R   0x1000"
check memory_test.stdout \
  "  LOAD           0x001004 0x0*01000 0x0*02004 0x0*04 0x0*04 R   0x1000"
check memory_test.stdout \
  "  LOAD           0x001008 0x0*02008 0x0*02008 0x0*08 0x0*08 R   0x1000"
check memory_test.stdout \
  "  LOAD           0x002000 0x0*05000 0x0*05000 0x0*04 0x0*04 R   0x1000"
check memory_test.stdout \
  "  LOAD           0x00203c 0x0*04000 0x0*0603c 0x0*04 0x0*04 R   0x1000"

exit 0
