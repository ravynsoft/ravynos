#!/bin/sh

# two_file_shared.sh -- check that debug info gets symbol addresses

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

addrs=`grep DW_OP_addr two_file_shared.dbg | fgrep '(DW_OP_addr: 0)'`
if test -n "$addrs"; then
  echo "Found variables with address zero"
  echo $addrs
  exit 1
fi
