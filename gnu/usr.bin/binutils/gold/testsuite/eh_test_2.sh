#!/bin/sh

# eh_test_2.sh -- check that .eh_frame_hdr is valid.

# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Written by Cary Coutant <ccoutant@gmail.com>.

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

sections="eh_test_2.sects"

hdr_section=`fgrep .eh_frame_hdr $sections`
size_field=`echo $hdr_section | sed -e 's/\[//' | awk '{print $6;}'`
size=`printf %d "0x$size_field"`

if test "$size" -le 8; then
  echo ".eh_frame_hdr section is too small:"
  echo "$hdr_section"
  exit 1
fi

exit 0
