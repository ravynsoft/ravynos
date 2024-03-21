#!/bin/sh

# split_i386.sh -- test -fstack-split for i386

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Ian Lance Taylor <iant@google.com>.

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

match()
{
  if ! $EGREP "$1" "$2" >/dev/null 2>&1; then
    echo 1>&2 "could not find '$1' in $2"
    exit 1
  fi
}

nomatch()
{
  if $EGREP "$1" "$2" >/dev/null 2>&1; then
    echo 1>&2 "found unexpected '$1' in $2"
    exit 1
  fi
}

match 'cmp.*+%gs:[^,]*,%esp' split_i386_1.stdout
match 'call.*__morestack>?$' split_i386_1.stdout
match 'lea.*-0x200\(%esp\),' split_i386_1.stdout

match 'stc' split_i386_2.stdout
match 'call.*__morestack_non_split>?$' split_i386_2.stdout
nomatch 'call.*__morestack>?$' split_i386_2.stdout
match 'lea.*-0x100200\(%esp\),' split_i386_2.stdout

match 'failed to match' split_i386_3.stdout

match 'call.*__morestack>?$' split_i386_4.stdout

match 'cannot mix' split_i386_r.stdout
