#! /usr/bin/sed -f
#
# Copyright (C) 2006 Free Software Foundation, Inc.
# Written by Bruno Haible <bruno@clisp.org>, 2006.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#
# Each line of the form ^...   .* contains the code for a language.
# The categorization in column 60 should be a space, denoting a living language,
# or an U.
# The speakers number in columns 62..69 should be at least a million.
#
/^...   ......................................................[ U]..[^ ]/ {
  h
  s/^...   \(.*[^ ]\)   .*$/\1./
  s/ç/@,{c}/g
  s/´/'/g
  x
  s/^\(...\).*/@item \1/
  G
  p
}
#
# delete the rest
#
d
