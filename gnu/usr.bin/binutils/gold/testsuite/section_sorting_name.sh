#!/bin/sh

# section_sorting_name.sh -- test

# Copyright (C) 2013-2023 Free Software Foundation, Inc.
# Written by Alexander Ivchenko <alexander.ivchenko@intel.com>.

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

# The goal of this program is to verify that when using --sort-section=name
# option all .text, .data, and .bss sections are sorted by name

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
	  printf \"layout of $2 and $3 is not right\\n\";
	  err = 1;
	  exit 1;
       }
    }
END {
      if (!saw1 && !err)
	{
	  printf \"did not see $2\\n\";
	  exit 1;
	}
      if (!saw2 && !err)
	{
	  printf \"did not see $3\\n\";
	  exit 1;
	}
    }" $1
}

# addr (hot_foo_0001) < addr (hot_foo_0002) < addr (hot_foo_0003)
check section_sorting_name.stdout "hot_foo_0001" "hot_foo_0002"
check section_sorting_name.stdout "hot_foo_0002" "hot_foo_0003"

check section_sorting_name.stdout "sorted_foo_0001" "sorted_foo_0001_abc"
check section_sorting_name.stdout "sorted_foo_0001_abc" "sorted_foo_0002"
check section_sorting_name.stdout "sorted_foo_0002" "sorted_foo_0003"
check section_sorting_name.stdout "sorted_foo_0003" "sorted_foo_y"
check section_sorting_name.stdout "sorted_foo_y" "sorted_foo_z"

check section_sorting_name.stdout "vdata_0001" "vdata_0002"
check section_sorting_name.stdout "vdata_0002" "vdata_0003"

check section_sorting_name.stdout "vbss_0001" "vbss_0002"
check section_sorting_name.stdout "vbss_0002" "vbss_0003"
