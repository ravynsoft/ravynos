#! /usr/bin/sed -f
#
# Copyright (C) 2001, 2003, 2006, 2022 Free Software Foundation, Inc.
# Written by Bruno Haible <bruno@clisp.org>, 2001.
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
# each line of the form ^..<tab>.* contains the code for a country.
#
/^..	/ {
  h
  s/^..	\(.*\)/\1./
  s/\&/and/g
  s/ô/@^{o}/g
  s/ü/@"{u}/g
  s/é/e/g
  s/Å/Aa/g
  x
  s/^\(..\).*/@item \1/
  G
  p
}
#
# delete the rest
#
d
