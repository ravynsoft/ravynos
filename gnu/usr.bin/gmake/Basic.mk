# Basic GNU -*-Makefile-*- to build GNU Make
#
# NOTE:
# If you have no 'make' program at all to process this makefile:
#   * On Windows, run ".\build_w32.bat" to bootstrap one.
#   * On MS-DOS, run ".\builddos.bat" to bootstrap one.
#
# Once you have a GNU Make program created, you can use it with this makefile
# to keep it up to date if you make changes, as:
#
#   make.exe -f Basic.mk
#
# Copyright (C) 2017-2023 Free Software Foundation, Inc.
# This file is part of GNU Make.
#
# GNU Make is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

all:

src = src/
lib = lib/

make_SOURCES = $(src)ar.c $(src)arscan.c $(src)commands.c $(src)default.c $(src)dir.c $(src)expand.c $(src)file.c $(src)function.c $(src)getopt.c $(src)getopt1.c $(src)guile.c $(src)hash.c $(src)implicit.c $(src)job.c $(src)load.c $(src)loadapi.c $(src)main.c $(src)misc.c $(src)output.c $(src)read.c $(src)remake.c $(src)rule.c $(src)shuffle.c $(src)signame.c $(src)strcache.c $(src)variable.c $(src)version.c $(src)vpath.c
glob_SOURCES = $(lib)fnmatch.c $(lib)glob.c
loadavg_SOURCES = $(lib)getloadavg.c
alloca_SOURCES = $(lib)alloca.c
w32_SOURCES = $(src)w32/pathstuff.c $(src)w32/w32os.c $(src)w32/compat/dirent.c $(src)w32/compat/posixfcn.c $(src)w32/subproc/misc.c $(src)w32/subproc/sub_proc.c $(src)w32/subproc/w32err.c
vms_SOURCES = $(src)vms_exit.c $(src)vms_export_symbol.c $(src)vms_progname.c $(src)vmsfunctions.c $(src)vmsify.c
amiga_SOURCES = $(src)amiga.c

remote_SOURCES = $(src)remote-stub.c

OUTDIR =
SRCDIR = .

OBJEXT = o
EXEEXT =

PREFIX = /usr/local
INCLUDEDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib
LOCALEDIR = $(PREFIX)/share

PROG = $(OUTDIR)make$(EXEEXT)

prog_SOURCES = $(make_SOURCES) $(remote_SOURCES)

BUILT_SOURCES =

OBJECTS = $(patsubst %.c,$(OUTDIR)%.$(OBJEXT),$(prog_SOURCES))

OBJDIRS = $(addsuffix .,$(sort $(dir $(OBJECTS))))

# Use the default value of CC
LD = $(CC)

# Reserved for command-line override
CPPFLAGS =
CFLAGS = -g -O2
LDFLAGS =

extra_CPPFLAGS = -DHAVE_CONFIG_H -I$(OUTDIR)src -I$(SRCDIR)/src -I$(OUTDIR)lib -I$(SRCDIR)/lib \
	-DLIBDIR=\"$(LIBDIR)\" -DINCLUDEDIR=\"$(INCLUDEDIR)\" -DLOCALEDIR=\"$(LOCALDIR)\"
extra_CFLAGS =
extra_LDFLAGS = $(extra_CFLAGS) $(CFLAGS)

C_SOURCE = -c
OUTPUT_OPTION = -o $@
LINK_OUTPUT = -o $@

# Command lines

# $(call COMPILE.cmd,<src>,<tgt>)
COMPILE.cmd = $(CC) $(extra_CFLAGS) $(CFLAGS) $(extra_CPPFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(OUTPUT_OPTION) $(C_SOURCE) $1

# $(call LINK.cmd,<objectlist>)
LINK.cmd = $(LD) $(extra_LDFLAGS) $(LDFLAGS) $(TARGET_ARCH) $1 $(LDLIBS) $(LINK_OUTPUT)

# $(CHECK.cmd) $(CHECK.args)
CHECK.cmd = cd $(SRCDIR)/tests && ./run_make_tests -make $(shell cd $(<D) && pwd)/$(<F)
CHECK.args ?=

# $(call MKDIR.cmd,<dirlist>)
MKDIR.cmd = mkdir -p $1

# $(call RM.cmd,<filelist>)
RM.cmd = rm -f $1

# $(call CP.cmd,<from>,<to>)
CP.cmd = cp $1 $2

CLEANSPACE = $(call RM.cmd,$(OBJECTS) $(PROG) $(BUILT_SOURCES))

# Load overrides for the above variables.
include $(firstword $(wildcard $(SRCDIR)/mk/$(lastword $(subst -, ,$(MAKE_HOST)).mk)))

VPATH = $(SRCDIR)

all: $(PROG)

$(PROG): $(OBJECTS)
	$(call LINK.cmd,$^)

$(OBJECTS): $(OUTDIR)%.$(OBJEXT): %.c
	$(call COMPILE.cmd,$<)

$(OBJECTS): | $(OBJDIRS) $(BUILT_SOURCES)

$(OBJDIRS):
	$(call MKDIR.cmd,$@)

check:
	$(CHECK.cmd) $(CHECK.args)

clean:
	$(CLEANSPACE)

$(filter %.h,$(BUILT_SOURCES)): %.h : %.in.h
	$(call RM.cmd,$@)
	$(call CP.cmd,$<,$@)

.PHONY: all check clean

# --------------- DEPENDENCIES
#

$(OBJECTS): $(SRCDIR)/src/mkconfig.h

# src/.deps/amiga.Po
# dummy

# src/.deps/ar.Po
$(OUTDIR)src/ar.$(OBJEXT): $(SRCDIR)/src/ar.c $(SRCDIR)/src/makeint.h $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h \
  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/dep.h $(SRCDIR)/lib/intprops.h $(SRCDIR)/lib/intprops-internal.h

# src/.deps/arscan.Po
$(OUTDIR)src/arscan.$(OBJEXT): $(SRCDIR)/src/arscan.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/lib/intprops.h $(SRCDIR)/lib/intprops-internal.h $(SRCDIR)/src/output.h

# src/.deps/commands.Po
$(OUTDIR)src/commands.$(OBJEXT): $(SRCDIR)/src/commands.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/os.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h

# src/.deps/default.Po
$(OUTDIR)src/default.$(OBJEXT): $(SRCDIR)/src/default.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/rule.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h \
 $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h

# src/.deps/dir.Po
$(OUTDIR)src/dir.$(OBJEXT): $(SRCDIR)/src/dir.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/hash.h $(SRCDIR)/src/filedef.h \
 $(SRCDIR)/src/dep.h $(SRCDIR)/src/debug.h \

# src/.deps/expand.Po
$(OUTDIR)src/expand.$(OBJEXT): $(SRCDIR)/src/expand.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/job.h \
 $(SRCDIR)/src/output.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/rule.h

# src/.deps/file.Po
$(OUTDIR)src/file.$(OBJEXT): $(SRCDIR)/src/file.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h \
 $(SRCDIR)/src/variable.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/shuffle.h

# src/.deps/function.Po
$(OUTDIR)src/function.$(OBJEXT): $(SRCDIR)/src/function.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/variable.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/os.h $(SRCDIR)/src/commands.h \
 $(SRCDIR)/src/debug.h

# src/.deps/getopt.Po
$(OUTDIR)src/getopt.$(OBJEXT): $(SRCDIR)/src/getopt.c $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h \
  \

# src/.deps/getopt1.Po
$(OUTDIR)src/getopt1.$(OBJEXT): $(SRCDIR)/src/getopt1.c $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h \
  \
 $(SRCDIR)/src/getopt.h \

# src/.deps/guile.Po
$(OUTDIR)src/guile.$(OBJEXT): $(SRCDIR)/src/guile.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/debug.h $(SRCDIR)/src/filedef.h \
 $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/variable.h \
 $(SRCDIR)/src/gmk-default.h

# src/.deps/hash.Po
$(OUTDIR)src/hash.$(OBJEXT): $(SRCDIR)/src/hash.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/hash.h \

# src/.deps/implicit.Po
$(OUTDIR)src/implicit.$(OBJEXT): $(SRCDIR)/src/implicit.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/rule.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/shuffle.h

# src/.deps/job.Po
$(OUTDIR)src/job.$(OBJEXT): $(SRCDIR)/src/job.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/os.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/shuffle.h \
 $(SRCDIR)/lib/findprog.h

# src/.deps/load.Po
$(OUTDIR)src/load.$(OBJEXT): $(SRCDIR)/src/load.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/debug.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/variable.h

# src/.deps/loadapi.Po
$(OUTDIR)src/loadapi.$(OBJEXT): $(SRCDIR)/src/loadapi.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/variable.h $(SRCDIR)/src/dep.h

# src/.deps/main.Po
$(OUTDIR)src/main.$(OBJEXT): $(SRCDIR)/src/main.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/os.h $(SRCDIR)/src/filedef.h \
 $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/rule.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/getopt.h $(SRCDIR)/src/shuffle.h \

# src/.deps/misc.Po
$(OUTDIR)src/misc.$(OBJEXT): $(SRCDIR)/src/misc.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/dep.h $(SRCDIR)/src/os.h $(SRCDIR)/src/debug.h \

# src/.deps/output.Po
$(OUTDIR)src/output.$(OBJEXT): $(SRCDIR)/src/output.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/os.h $(SRCDIR)/src/output.h \

# src/.deps/posixos.Po
$(OUTDIR)src/posixos.$(OBJEXT): $(SRCDIR)/src/posixos.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/debug.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/os.h

# src/.deps/read.Po
$(OUTDIR)src/read.$(OBJEXT): $(SRCDIR)/src/read.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/os.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/rule.h $(SRCDIR)/src/debug.h

# src/.deps/remake.Po
$(OUTDIR)src/remake.$(OBJEXT): $(SRCDIR)/src/remake.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/variable.h \
 $(SRCDIR)/src/debug.h \

# src/.deps/remote-cstms.Po
# dummy

# src/.deps/remote-stub.Po
$(OUTDIR)src/remote-stub.$(OBJEXT): $(SRCDIR)/src/remote-stub.c \
 $(SRCDIR)/src/makeint.h $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h \
  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h

# src/.deps/rule.Po
$(OUTDIR)src/rule.$(OBJEXT): $(SRCDIR)/src/rule.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h $(SRCDIR)/src/commands.h \
 $(SRCDIR)/src/variable.h $(SRCDIR)/src/rule.h

# src/.deps/shuffle.Po
$(OUTDIR)src/shuffle.$(OBJEXT): $(SRCDIR)/src/shuffle.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/shuffle.h $(SRCDIR)/src/filedef.h \
 $(SRCDIR)/src/hash.h $(SRCDIR)/src/dep.h

# src/.deps/signame.Po
$(OUTDIR)src/signame.$(OBJEXT): $(SRCDIR)/src/signame.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \

# src/.deps/strcache.Po
$(OUTDIR)src/strcache.$(OBJEXT): $(SRCDIR)/src/strcache.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/hash.h

# src/.deps/variable.Po
$(OUTDIR)src/variable.$(OBJEXT): $(SRCDIR)/src/variable.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h $(SRCDIR)/src/debug.h $(SRCDIR)/src/dep.h $(SRCDIR)/src/job.h $(SRCDIR)/src/output.h \
 $(SRCDIR)/src/commands.h $(SRCDIR)/src/variable.h $(SRCDIR)/src/os.h $(SRCDIR)/src/rule.h

# src/.deps/version.Po
$(OUTDIR)src/version.$(OBJEXT): $(SRCDIR)/src/version.c $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h \
 

# src/.deps/vms_exit.Po
# dummy

# src/.deps/vms_export_symbol.Po
# dummy

# src/.deps/vms_progname.Po
# dummy

# src/.deps/vmsfunctions.Po
# dummy

# src/.deps/vmsify.Po
# dummy

# src/.deps/vpath.Po
$(OUTDIR)src/vpath.$(OBJEXT): $(SRCDIR)/src/vpath.c $(SRCDIR)/src/makeint.h \
 $(OUTDIR)src/config.h \
 $(SRCDIR)/src/../src/mkcustom.h  \
 $(SRCDIR)/src/gnumake.h \
 $(SRCDIR)/src/gettext.h \
 $(SRCDIR)/src/filedef.h $(SRCDIR)/src/hash.h \
 $(SRCDIR)/src/variable.h
