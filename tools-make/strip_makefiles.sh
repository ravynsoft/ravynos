#!/bin/sh
# strip_makefiles.sh
#
# Copyright (C) 2003 Free Software Foundation, Inc.
#
# Author: Nicola Pero <n.pero@mi.flashnet.it>
# Date: October 2003
#
# This file is part of the GNUstep Makefile Package.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
# 
# You should have received a copy of the GNU General Public
# License along with this library; see the file COPYING.
# If not, write to the Free Software Foundation,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# This script "strips" the makefiles and shell scripts.

# By "stripping" a makefile we mean removing all comments, and all
# empty lines.  This reduces considerably the size of makefiles and
# the amount of data that each make invocation has to read; the
# makefiles execute slightly faster.  You shouldn't over-estimate the
# performance issue though - stripped makefiles execute only 5% faster
# on my machine.

# The disadvantage of stripped makefiles is that we remove comments
# and makefiles become almost unreadable.

for makefile in *.make Master/*.make Instance/*.make Instance/Shared/*.make Instance/Documentation/*.make; do
  sed -e '/^ *#/d' -e '/^$/d' ${makefile} > ${makefile}.stripped;
  mv ${makefile}.stripped ${makefile};
done

for shell_script in *.sh *.csh; do
  sed -e '/^ *#/d' -e '/^$/d' ${shell_script} > ${shell_script}.stripped;
  mv ${shell_script}.stripped ${shell_script};
  chmod 755 ${shell_script};
done

