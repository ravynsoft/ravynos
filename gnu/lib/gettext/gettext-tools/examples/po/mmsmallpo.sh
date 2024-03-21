#!/bin/sh
#
# Copyright (C) 2003-2004, 2009 Free Software Foundation, Inc.
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

# Usage: mmsmallpo.sh hello-foo ll

set -e

test $# = 2  || { echo "Usage: mmsmallpo.sh hello-foo ll" 1>&2; exit 1; }
directory=$1
language=$2

msgmerge --quiet --force-po $language.po $directory.pot -o - | \
msgattrib --no-obsolete | \
sed -e "s, $directory/, ,g" | sed -e "s,gettext-examples,$directory," | \
sed -e '/^"POT-Creation-Date: .*"$/{
x
s/P/P/
ta
g
d
bb
:a
x
:b
}' \
  > ../$directory/po/$language.po
