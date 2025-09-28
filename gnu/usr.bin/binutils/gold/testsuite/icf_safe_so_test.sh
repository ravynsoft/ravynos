#!/bin/sh

# icf_safe_so_test.sh -- test --icf=safe

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
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

# The goal of this program is to verify if --icf=safe  works as expected.
# File icf_safe_so_test.cc is in this test.  The goal of this script is
# to verify if identical code folding in safe mode correctly folds
# functions in a shared object.

set -e

error_if_symbol_absent()
{
    if ! is_symbol_present $1 $2;
    then
      echo "Symbol" $2 "not present, possibly folded."
      exit 1
    fi
}

is_symbol_present()
{
    grep $2 $1 > /dev/null 2>&1
    return $?
}

check_nofold()
{
    error_if_symbol_absent $1 $2
    error_if_symbol_absent $1 $3
    func_addr_1=`grep $2 $1 | awk '{print $1}'`
    func_addr_2=`grep $3 $1 | awk '{print $1}'`
    if [ $func_addr_1 = $func_addr_2 ];
    then
        echo "Safe Identical Code Folding folded" $2 "and" $3
	exit 1
    fi
}

check_fold()
{
    map=$1
    shift
    num_syms=$#
    save_IFS="$IFS"
    IFS='|'
    sym_patt="$*"
    IFS="$save_IFS"
    awk "
BEGIN { discard = 0; }
/^Discarded input/ { discard = 1; }
/^Memory map/ { discard = 0; }
/.*\\.text\\..*($sym_patt).*/ { act[discard] = act[discard] \" \" \$0; cnt[discard] = cnt[discard] + 1 }
END {
      printf \"kept\" act[0] \"\\nfolded\" act[1] \"\\n\";
      if (cnt[0] != 1 || cnt[1] != $num_syms - 1)
	{
	  printf \"Safe Identical Code Folding failed\\n\"
	  exit 1;
	}
    }" $map
}

arch_specific_safe_fold()
{
    if grep -q -e "Advanced Micro Devices X86-64" -e "Intel 80386" -e "ARM" -e "PowerPC" $1;
    then
	shift
	shift
	#echo check_fold $*
	check_fold $*
    else
	shift
	nm_output=$1
	shift
	shift
	while test $# -gt 1; do
	    sym1=$1
	    shift
	    for sym2 in $*; do
		#echo check_nofold $nm_output $sym1 $sym2
		check_nofold $nm_output $sym1 $sym2
	    done
	done
    fi
}

arch_specific_safe_fold icf_safe_so_test_2.stdout icf_safe_so_test_1.stdout icf_safe_so_test.map foo_prot foo_hidden foo_internal foo_static
check_nofold icf_safe_so_test_1.stdout foo_glob bar_glob
