#!/bin/sh

# ver_test_1.sh -- check that protected symbols are local

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

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

syms=`grep ' HIDDEN ' ver_test_1.syms | grep ' GLOBAL '`
if test -n "$syms"; then
  echo "Found GLOBAL HIDDEN symbols"
  echo $syms
  exit 1
fi
