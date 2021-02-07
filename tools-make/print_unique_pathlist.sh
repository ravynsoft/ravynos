#! /bin/sh
#
# print_unique_path_list.sh
#
#   Script for creating unique lists of paths
#
#   Copyright (C) 2007 Free Software Foundation, Inc.
#
#   Author: Nicola Pero <nicola.pero@meta-innovation.com
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# If we knew the shell is bash, we could easily put this code into a
# function.  (FIXME: check if there is a portable way of doing that)

# This is a shell script that takes 5 arguments (a list of 4 paths,
# followed by "yes" if it needs to automatically convert the paths
# from Win32 to Unix style), and prints the 4 paths separated by ':',
# removing duplicates.

# The order of the paths printed is the REVERSE of the order in which
# they were supplied, this is so that code which iterates through them
# prepending them to a PATH will leave them in the correct order in
# that path.

# GNUSTEP_MAKEFILES needs to be defined when you execute this script.

if [ ! $# -eq 5 ]; then
  echo "Usage: $0 path1 path2 path3 path4 windowsToUnixConversion"
  echo " prints out path1:path2:path3:path4 removing duplicates"
  echo " and converting paths from windows to unix if windowsToUnixConversion"
  echo " is 'yes'"
  exit 1
fi

GS_MAKE_RESULT=""

if [ "$5" = "yes" ]; then
  GS_MAKE_PATH_1=`$GNUSTEP_MAKEFILES/fixpath.sh -u "$1"`
  GS_MAKE_PATH_2=`$GNUSTEP_MAKEFILES/fixpath.sh -u "$2"`
  GS_MAKE_PATH_3=`$GNUSTEP_MAKEFILES/fixpath.sh -u "$3"`
  GS_MAKE_PATH_4=`$GNUSTEP_MAKEFILES/fixpath.sh -u "$4"`
else
  GS_MAKE_PATH_1="$1"
  GS_MAKE_PATH_2="$2"
  GS_MAKE_PATH_3="$3"
  GS_MAKE_PATH_4="$4"
fi

# Now we basically want to build
# GS_MAKE_RESULT="$GS_MAKE_PATH_1:$GS_MAKE_PATH_2:$GS_MAKE_PATH_3:$GS_MAKE_PATH_4"
# but we want to remove duplicates.

# Start with $GS_MAKE_PATH_1:$GS_MAKE_PATH_2 - or $GS_MAKE_PATH_1 if they are the same
if [ "$GS_MAKE_PATH_2" != "$GS_MAKE_PATH_1" ]; then
  GS_MAKE_RESULT="$GS_MAKE_PATH_1:$GS_MAKE_PATH_2"
else
  GS_MAKE_RESULT="$GS_MAKE_PATH_1"
fi

# Now append $GS_MAKE_PATH_3 but only if different from what already there
if [ "$GS_MAKE_PATH_3" != "$GS_MAKE_PATH_1" ]; then
  if [ "$GS_MAKE_PATH_3" != "$GS_MAKE_PATH_2" ]; then
    GS_MAKE_RESULT="$GS_MAKE_RESULT:$GS_MAKE_PATH_3"
  fi
fi

# Now append $GS_MAKE_PATH_4 but only if different from what already there
if [ "$GS_MAKE_PATH_4" != "$GS_MAKE_PATH_1" ]; then
  if [ "$GS_MAKE_PATH_4" != "$GS_MAKE_PATH_2" ]; then
    if [ "$GS_MAKE_PATH_4" != "$GS_MAKE_PATH_3" ]; then
      GS_MAKE_RESULT="$GS_MAKE_RESULT:$GS_MAKE_PATH_4"
    fi
  fi
fi

# Now reverse the order of the path fragments so that the calling code
# can iterate through them and prepend each in turn to another path.
old_IFS="$IFS"
IFS=:
reversed=""
for dir in $GS_MAKE_RESULT; do
  path_fragment="$dir"
  if [ -z "$reversed" ]; then
    reversed="$path_fragment"
  else
    reversed="$path_fragment:$reversed"
  fi
done
GS_MAKE_RESULT="$reversed"
IFS="$old_IFS"
unset reversed
unset old_IFS

echo "$GS_MAKE_RESULT"
