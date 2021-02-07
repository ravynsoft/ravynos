#   -*-makefile-*-
#   serial-subdirectories.make
#
#   Master Makefile rules to build a set of subdirectories in serial sequence.
#
#   Copyright (C) 2010 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
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

ifeq ($(RULES_MAKE_LOADED),)
  include $(GNUSTEP_MAKEFILES)/rules.make
endif

# The list of subdirectories is in the makefile variable
# SERIAL_SUBDIRECTORIES.
SERIAL_SUBDIRECTORIES := $(strip $(SERIAL_SUBDIRECTORIES))

# This file iterates over the SERIAL_SUBDIRECTORIES, in order, steps
# in each directory in turn, and runs a submake in there.  The project
# types in the directories can be anything - tools, documentation,
# libraries, bundles, applications, whatever.  For example, if your
# package is composed by a library and then by some tools using the
# library, you could have the library in one directory, the tools in
# another directory, and have a top level GNUmakefile which has the
# two as SERIAL_SUBDIRECTORIES; they will be built in the specified
# order.
#
# If you do not require the subdirectories to be built in strict
# order, you can use parallel-subdirectories.make which will
# parallelize the build of the subdirectories if possible.


ifneq ($(SERIAL_SUBDIRECTORIES),)

  # We use a subshell and do the job there to minimize the number of processes we need to spawn
  # to perform this job.  Note that we don't need to pass _GNUSTEP_MAKE_PARALLEL=no to it (as this
  # shouldn't be a parallel Master invocation), but we do it anyway for safety.
  internal-all internal-install internal-uninstall \
  internal-clean internal-distclean \
  internal-check internal-strings::
	$(ECHO_NOTHING_RECURSIVE_MAKE)operation=$(subst internal-,,$@); \
	  abs_build_dir="$(ABS_GNUSTEP_BUILD_DIR)"; \
	for directory in $(SERIAL_SUBDIRECTORIES); do \
	  $(INSIDE_ECHO_MAKING_OPERATION_IN_DIRECTORY) \
	  if [ "$${abs_build_dir}" = "." ]; then \
	    gsbuild="."; \
	  else \
	    gsbuild="$${abs_build_dir}/$$directory"; \
	  fi; \
	  if $(MAKE) -C $$directory -f $(MAKEFILE_NAME) $(GNUSTEP_MAKE_NO_PRINT_DIRECTORY_FLAG) --no-keep-going \
                     $$operation \
	             GNUSTEP_BUILD_DIR="$$gsbuild" _GNUSTEP_MAKE_PARALLEL=no; then \
	    :; else exit $$?; \
	  fi; \
	done$(END_ECHO_RECURSIVE_MAKE)

endif
