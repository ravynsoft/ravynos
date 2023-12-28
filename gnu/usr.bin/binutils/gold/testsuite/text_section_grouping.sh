#!/bin/sh

# text_section_grouping.sh -- test

# Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

# The goal of this program is to verify if .text sections are grouped
# according to prefix.  .text.unlikely, .text.startup and .text.hot should
# be grouped and placed together.

# Also check if the functions do not get grouped with option --no-text-reorder.

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

# addr (unlikely_*) < addr (startup_*) < addr (hot_*)
check text_section_grouping.stdout "unlikely_foo" "startup_foo"
check text_section_grouping.stdout "startup_foo" "hot_foo"
check text_section_grouping.stdout "unlikely_bar" "startup_bar"
check text_section_grouping.stdout "startup_bar" "hot_bar"
check text_section_grouping.stdout "unlikely_foo" "startup_bar"
check text_section_grouping.stdout "startup_foo" "hot_bar"

check text_section_no_grouping.stdout "hot_foo" "startup_foo"
check text_section_no_grouping.stdout "startup_foo" "unlikely_foo"
check text_section_no_grouping.stdout "unlikely_foo" "hot_bar"
check text_section_no_grouping.stdout "hot_bar" "startup_bar"
check text_section_no_grouping.stdout "startup_bar" "unlikely_bar"
