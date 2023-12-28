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

TARGETS     = synprog so_syn.so so_syx.so
TARGET      = ./synprog
ACCT_FILE   = synprog.acct

srcdir = .
include $(srcdir)/../../lib/Makefile.skel

SRCS = \
	$(srcdir)/../mttest/gethrtime.c \
	$(srcdir)/synprog.c \
	$(srcdir)/callso.c \
	$(srcdir)/callsx.c \
	$(srcdir)/endcases.c \
	$(srcdir)/fitos.c \
	$(srcdir)/iosyn.c \
	$(srcdir)/pagethrash.c \
	$(srcdir)/stopwatch.c \
	$(NULL)

HDRS= \
	$(srcdir)/inc_body.h \
	$(srcdir)/inc_brace.h \
	$(srcdir)/inc_entry.h \
	$(srcdir)/inc_exit.h \
	$(srcdir)/inc_func.h \
	$(srcdir)/inc_inline.h \
	$(srcdir)/inc_macro.h \
	$(srcdir)/stopwatch.h \
	$(NULL)


$(TARGET): $(SRCS) $(HDRS) so_syx.so so_syn.so
	@echo " ---- Build: $@ -----"
	$(CC) $(CFLAGS) -Wl,-E -o $@ $(SRCS) -ldl -lc -lrt

so_syx.so: $(srcdir)/so_syx.c
	@echo " ---- Build: $@ -----"
	$(CC) $(CFLAGS) $(SHAREDOPT) -o $@ $(srcdir)/so_syx.c -lc

so_syn.so: $(srcdir)/so_syn.c
	@echo " ---- Build: $@ -----"
	$(CC) $(CFLAGS) $(SHAREDOPT) -o $@ $(srcdir)/so_syn.c -lc

$(EXPERIMENT): $(TARGETS)
	rm -rf $@
	$(COLLECT) $(COLLECT_FLAGS) -o $@ $(TARGET) $(TARGET_FLAGS)

