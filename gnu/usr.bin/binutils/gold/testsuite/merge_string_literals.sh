#!/bin/sh

# merge_string_literals.sh -- test

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

# The goal of this program is to check whether string literals from different
# object files are merged together

set -e

check()
{
    number_of_occurrence=`grep $2 ./$1 -o| wc -l`
    if [ $number_of_occurrence != $3 ]
    then
	echo "String literals were not merged"
	exit 1
    fi
}

# If string literals were merged, then "abcd" appears two times
check merge_string_literals.stdout "abcd" 2
