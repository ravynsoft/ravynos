#!/bin/sh

# plugin_final_layout.sh -- test

# Copyright (C) 2011-2023 Free Software Foundation, Inc.
# Written by Sriraman Tallam <tmsriram@google.com>.

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

# The goal of this program is to verify if -z,text-unlikely-segment works as
# intended.  File text_unlikely_segment.cc is part of this test.


set -e

# With readelf -l, an ELF Section to Segment mapping is printed as :
##############################################
#  Section to Segment mapping:
#  Segment Sections...
#  ...
#     0x     .text.unlikely
#  ...
##############################################
# Check if .text.unlikely is the only section in the segment.
check_unique_segment()
{
    awk "
BEGIN { saw_section = 0; saw_unique = 0; }
/$2/ { saw_section = 1; }
/[ ]*0[0-9][ ]*$2[ ]*\$/ { saw_unique = 1; }
END {
      if (!saw_section)
	{
	  printf \"Section $2 not seen in output\\n\";
	  exit 1;
	}
      else if (!saw_unique)
	{
	  printf \"Unique segment not seen for: $2\\n\";
	  exit 1;
	}
    }" $1
}

check_unique_segment text_unlikely_segment_readelf.stdout ".text.unlikely"
