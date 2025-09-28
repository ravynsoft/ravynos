#!/bin/sh

# icf_sht_rel_addend_test.sh -- test --icf=all

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


check()
{
    func_addr_1=`grep $2 $1 | awk '{print $1}'`
    func_addr_2=`grep $3 $1 | awk '{print $1}'`
    if [ $func_addr_1 = $func_addr_2 ]
    then
        echo "Identical Code Folding should not fold" $2 "and" $3
	exit 1
    fi
}

check icf_sht_rel_addend_test.stdout "name1" "name2"
