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

# The goal of this program is to verify if --section-ordering-file works as
# intended.  File plugin_final_layout.cc is in this test.


set -e

check()
{
    awk "
BEGIN { saw1 = 0; saw2 = 0; err = 0; }
/.*$2\$/ { saw1 = 1; }
/.*$3\$/ {
     saw2 = 1;
     if (!saw1)
       {
	  printf \"layout of $2 and $3 is not right in file $1\\n\";
	  err = 1;
	  exit 1;
       }
    }
END {
      if (!saw1 && !err)
        {
	  printf \"did not see $2 in file $1\\n\";
	  exit 1;
	}
      if (!saw2 && !err)
	{
	  printf \"did not see $3 in file $1\\n\";
	  exit 1;
	}
    }" $1
}

# With readelf -l, an ELF Section to Segment mapping is printed as :
##############################################
#  Section to Segment mapping:
#  Segment Sections...
#  ...
#     0x     .text.plugin_created_unique
#  ...
##############################################
# Check of .text.plugin_created_unique is the only section in the segment.
check_unique_segment()
{
    awk "
BEGIN { saw_section = 0; saw_unique = 0; }
/$2/ { saw_section = 1; }
/[ ]*0[0-9][ ]*$2[ ]*\$/ { saw_unique = 1; }
END {
      if (!saw_section)
	{
	  printf \"Section $2 not seen in output file $1\\n\";
	  exit 1;
	}
      else if (!saw_unique)
	{
	  printf \"Unique segment not seen for: $2 in file $1\\n\";
	  exit 1;
	}
    }" $1
}

check plugin_final_layout.stdout "_Z3foov" "_Z3barv"
check plugin_final_layout.stdout "_Z3barv" "_Z3bazv"
check_unique_segment plugin_final_layout_readelf.stdout ".text.plugin_created_unique"

check plugin_layout_new_file.stdout "_Z3foov" "_Z3barv"
check plugin_layout_new_file.stdout "_Z3barv" "_Z3bazv"
check_unique_segment plugin_layout_new_file_readelf.stdout ".text.plugin_created_unique"
