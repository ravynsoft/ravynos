#   Copyright (C) 2021-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This file is free software; you can redistribute it and/or modify
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

# This Makefile builds the mttest demo code

# Select a thread flag, BOUND or UNBOUND, and comment the other one
#FLAG	 = UNBOUND
FLAG	= BOUND

TARGETS	    = ./mttest
TARGET	    = ./mttest
ACCT_FILE   = mttest.acct

srcdir	    = .
include $(srcdir)/../../lib/Makefile.skel

SRCS	    = $(srcdir)/gethrtime.c $(srcdir)/mttest.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -D$(FLAG) -pthread -o $@ $(SRCS)

$(EXPERIMENT): $(TARGETS)
	rm -rf $@
	$(COLLECT) $(COLLECT_FLAGS) -o $@ $(TARGET) $(TARGET_FLAGS)

