#!/bin/bash
#
# spin.bash -- provide a `spinning wheel' to show progress
#
# Chet Ramey
# chet@po.cwru.edu
#
#  Copyright 1997 Chester Ramey
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   TThis program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software Foundation,
#   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

bs=$'\b'
 
chars="|${bs} \\${bs} -${bs} /${bs}"
 
# Infinite loop for demo. purposes
while :
do
    for letter in $chars
    do
        echo -n ${letter}
    done
done

exit 0
