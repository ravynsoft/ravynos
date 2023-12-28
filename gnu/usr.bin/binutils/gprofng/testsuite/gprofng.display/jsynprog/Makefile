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

TARGETS     = libcloop.so jsynprog.class
TARGET      = jsynprog
ACCT_FILE   = jsynprog.acct

srcdir = .
include $(srcdir)/../../lib/Makefile.skel

JAVACFLAGS  =

SRCS = \
	$(srcdir)/../mttest/gethrtime.c \
	$(srcdir)/cloop.cc \
	$(NULL)

JAVA_SRCS = \
	$(srcdir)/Intface.java \
	$(srcdir)/Routine.java \
	$(srcdir)/Sub_Routine.java \
	$(srcdir)/jsynprog.java \
	$(srcdir)/Launcher.java \
	$(NULL)

HDRS = jsynprog.h

libcloop.so: $(SRCS) 
	@echo " ---- Build: $@ -----"
	$(CC) $(jdk_inc) $(CCOPTS) $(SHAREDOPT) -o $@ $(SRCS)

jsynprog.class: $(JAVA_SRCS)
	@echo " ---- Build: $@ -----"
	$(JAVAC) $(JAVACFLAGS) -d . $(JAVA_SRCS)

$(EXPERIMENT): $(TARGETS)
	@echo " ---- Build: $@ -----"
	rm -rf $@ 
	$(COLLECT) $(COLLECT_FLAGS) -o $@ $(JAVA) $(JAVACFLAGS) jsynprog $(TARGET_FLAGS)

