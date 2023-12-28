#!/bin/sh

# icf_test_pr21066.sh -- regression test for ICF tracking exception handling
# metadata differences

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Joshua Oreman <oremanj@hudson-trading.com>, based on icf_test.sh

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

check()
{
    awk "
BEGIN { discard = 0; }
/^Discarded input/ { discard = 1; }
/^Memory map/ { discard = 0; }
/.*\\.text\\..*capture_exception_of_type.*($2|$3).*/ {
      act[discard] = act[discard] \" \" \$0;
}
END {
      # printf \"kept\" act[0] \"\\nfolded\" act[1] \"\\n\";
      if (length(act[0]) != 0 && length(act[1]) != 0)
	{
	  printf \"Identical Code Folding improperly folded functions\\n\"
	  printf \"with same code but different .gcc_except_table\\n\"
	  exit 1;
	}
    }" $1
}

check icf_test_pr21066.map "first_exception" "second_exception"
