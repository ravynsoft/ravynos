#!/bin/sh

# i386_mov_to_lea.sh -- a test for mov2lea conversion.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Tocar Ilya <ilya.tocar@intel.com>

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

set -e

grep -q "lea    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea1.stdout
grep -q "lea    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea2.stdout
grep -q "lea    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea3.stdout
grep -q "mov    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea4.stdout
grep -q "lea    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea5.stdout
grep -q "mov    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea6.stdout
grep -q "lea    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea7.stdout
grep -q "mov    -0x[a-f0-9]\+(%ecx),%eax" i386_mov_to_lea8.stdout

exit 0
