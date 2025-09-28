#!/bin/sh

# icf_safe_pie_test.sh -- test --icf=safe -pie

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Sriraman Tallam <tmsriram@google.com>.
# Modified by Rahul Chaudhry <rahulchaudhry@google.com>.

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

# The goal of this program is to verify if --icf=safe works with
# -pie as expected. File icf_safe_test.cc is in this test. This
# program checks if only ctors and dtors are folded, except for
# the architectures which use relocation types and instruction
# opcodes to detect if function pointers are taken.

set -e

check_nofold()
{
    func_addr_1=`grep $2 $1 | awk '{print $1}'`
    func_addr_2=`grep $3 $1 | awk '{print $1}'`
    if [ $func_addr_1 = $func_addr_2 ]
    then
        echo "Safe Identical Code Folding folded" $2 "and" $3
	exit 1
    fi
}

check_fold()
{
    awk "
BEGIN { discard = 0; }
/^Discarded input/ { discard = 1; }
/^Memory map/ { discard = 0; }
/.*\\.text\\..*($2|$3).*/ { act[discard] = act[discard] \" \" \$0; }
END {
      # printf \"kept\" act[0] \"\\nfolded\" act[1] \"\\n\";
      if (length(act[0]) == 0 || length(act[1]) == 0)
	{
	  printf \"Safe Identical Code Folding did not fold $2 and $3\\n\"
	  exit 1;
	}
    }" $1
}

arch_specific_safe_fold()
{
    if grep -q -e "Advanced Micro Devices X86-64" -e "Intel 80386" -e "ARM" -e "TILE" -e "PowerPC" -e "AArch64" -e "IBM S/390" $2;
    then
      check_fold $3 $4 $5
    else
      check_nofold $1 $4 $5
    fi
}

arch_specific_safe_fold icf_safe_pie_test_1.stdout icf_safe_pie_test_2.stdout \
  icf_safe_pie_test.map "kept_func_1" "kept_func_2"
check_fold   icf_safe_pie_test.map "_ZN1AD2Ev" "_ZN1AC2Ev"
check_nofold icf_safe_pie_test_1.stdout "kept_func_3" "kept_func_1"
check_nofold icf_safe_pie_test_1.stdout "kept_func_3" "kept_func_2"
