#!/bin/sh

# strong_ref_weak_def.sh -- test non-weak reference to a weak symbol defined
# in a DSO.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Doug Kwan <dougkwan@google.com>.

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

# This checks that the reference to 'weak_def' have GLOBAL binding.

check()
{
    file=$1
    pattern=$2
    found=`grep "$pattern" $file`
    if test -z "$found"; then
        echo "pattern \"$pattern\" not found in file $file."
	echo $found
        exit 1
    fi
}

check strong_ref_weak_def.stdout ".* FUNC.* GLOBAL.* UND.* weak_def"

exit 0
