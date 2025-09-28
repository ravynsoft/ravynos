#!/bin/sh

# split_s390.sh -- test -fstack-split for s390

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
# Written by Marcin Kościelnicki <koriakin@0x04.net>.

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

match 'jg.*__morestack>?$' split_s390_z1.stdout
match 'long.*0x00100000$' split_s390_z1.stdout
match 'jg.*__morestack>?$' split_s390_z1_ns.stdout
match 'long.*0x00104000$' split_s390_z1_ns.stdout

match 'ear.*$' split_s390_z2.stdout
match 'jgl.*__morestack>?$' split_s390_z2.stdout
nomatch 'jg	.*__morestack>?$' split_s390_z2.stdout
match 'long.*0x00000100$' split_s390_z2.stdout
nomatch 'ear.*$' split_s390_z2_ns.stdout
nomatch 'jgl.*__morestack>?$' split_s390_z2_ns.stdout
match 'jg	.*__morestack>?$' split_s390_z2_ns.stdout
match 'long.*0x00004100$' split_s390_z2_ns.stdout

match 'ear.*$' split_s390_z3.stdout
match 'jgl.*__morestack>?$' split_s390_z3.stdout
nomatch 'jg	.*__morestack>?$' split_s390_z3.stdout
match 'long.*0x00001000$' split_s390_z3.stdout
nomatch 'ear.*$' split_s390_z3_ns.stdout
nomatch 'jgl.*__morestack>?$' split_s390_z3_ns.stdout
match 'jg	.*__morestack>?$' split_s390_z3_ns.stdout
match 'long.*0x00005000$' split_s390_z3_ns.stdout

match 'alfi.*%r1,1048576$' split_s390_z4.stdout
match 'jgl.*__morestack>?$' split_s390_z4.stdout
match 'long.*0x00100000$' split_s390_z4.stdout
match 'alfi.*%r1,1064960$' split_s390_z4_ns.stdout
match 'jgl.*__morestack>?$' split_s390_z4_ns.stdout
match 'long.*0x00104000$' split_s390_z4_ns.stdout

match 'jg.*__morestack>?$' split_s390x_z1.stdout
match 'long.*0x00100000$' split_s390x_z1.stdout
match 'jg.*__morestack>?$' split_s390x_z1_ns.stdout
match 'long.*0x00104000$' split_s390x_z1_ns.stdout

match 'ear.*$' split_s390x_z2.stdout
match 'jgl.*__morestack>?$' split_s390x_z2.stdout
nomatch 'jg	.*__morestack>?$' split_s390x_z2.stdout
match 'long.*0x00000100$' split_s390x_z2.stdout
nomatch 'ear.*$' split_s390x_z2_ns.stdout
nomatch 'jgl.*__morestack>?$' split_s390x_z2_ns.stdout
match 'jg	.*__morestack>?$' split_s390x_z2_ns.stdout
match 'long.*0x00004100$' split_s390x_z2_ns.stdout

match 'ear.*$' split_s390x_z3.stdout
match 'jgl.*__morestack>?$' split_s390x_z3.stdout
nomatch 'jg	.*__morestack>?$' split_s390x_z3.stdout
match 'long.*0x00001000$' split_s390x_z3.stdout
nomatch 'ear.*$' split_s390x_z3_ns.stdout
nomatch 'jgl.*__morestack>?$' split_s390x_z3_ns.stdout
match 'jg	.*__morestack>?$' split_s390x_z3_ns.stdout
match 'long.*0x00005000$' split_s390x_z3_ns.stdout

match 'algfi.*%r1,1048576$' split_s390x_z4.stdout
match 'jgl.*__morestack>?$' split_s390x_z4.stdout
match 'long.*0x00100000$' split_s390x_z4.stdout
match 'algfi.*%r1,1064960$' split_s390x_z4_ns.stdout
match 'jgl.*__morestack>?$' split_s390x_z4_ns.stdout
match 'long.*0x00104000$' split_s390x_z4_ns.stdout

match 'larl' split_s390_n1.stdout
match 'larl' split_s390_n1_ns.stdout
match 'larl' split_s390x_n1.stdout
match 'larl' split_s390x_n1_ns.stdout
match 'j.*fn2' split_s390_n2.stdout
match 'j.*fn2' split_s390x_n2.stdout
match 'failed to match' split_s390_n2_ns.stdout
match 'failed to match' split_s390x_n2_ns.stdout

match 'failed to match' split_s390_a1.stdout
match 'failed to match' split_s390x_a1.stdout

match 'brasl.*__morestack>?$' split_s390_a2.stdout
match 'brasl.*__morestack>?$' split_s390x_a2.stdout

match 'cannot mix' split_s390_r.stdout
match 'cannot mix' split_s390x_r.stdout
