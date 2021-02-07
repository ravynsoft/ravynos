#   -*-makefile-*-
#   parallel-subdirectories.make
#
#   Master Makefile rules to build a set of subdirectories in parallel.
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
# PARALLEL_SUBDIRECTORIES.
PARALLEL_SUBDIRECTORIES := $(strip $(PARALLEL_SUBDIRECTORIES))

# This file builds all the PARALLEL_SUBDIRECTORIES in parallel.  It
# fires off a parallel submake invocation which starts a submake in
# each of the subdirectories.  These submakes are started in parallel
# if a parallel build is being done.  The project types in the
# directories can be anything - tools, documentation, libraries,
# bundles, applications, whatever.  For example, if your package is
# composed by a bunch of applications and tools that can be built
# independently of each other, you can simply put them into various
# subdirectories, then have a top level GNUmakefile which has them as
# PARALLEL_SUBDIRECTORIES; they will be built in parallel, if possible
# (or in a random serial order if parallel building is not used).
#
# If you do require the subdirectories to be built in strict order,
# you can use serial-subdirectories.make which will build them in the
# specified order.


ifneq ($(PARALLEL_SUBDIRECTORIES),)

  # We fire off a submake with _GNUSTEP_MAKE_PARALLEL=yes, so that that
  # submake invocation will be able to build its
  # 'internal-master-subdirectories-xxx' target in parallel.  If the
  # build is not parallel, that submake will simply build normally.
  internal-all internal-clean internal-distclean \
  internal-check internal-strings::
	$(ECHO_NOTHING_RECURSIVE_MAKE)operation=$(subst internal-,,$@); \
	  $(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	  internal-master-subdirectories-$$operation \
	  GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	  _GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

  .PHONY: \
    internal-master-subdirectories-all \
    internal-master-subdirectories-clean \
    internal-master-subdirectories-distclean \
    internal-master-subdirectories-check \
    internal-master-subdirectories-strings

  internal-master-subdirectories-all: $(PARALLEL_SUBDIRECTORIES:=.all.subdirectories)
  internal-master-subdirectories-clean: $(PARALLEL_SUBDIRECTORIES:=.clean.subdirectories)
  internal-master-subdirectories-distclean: $(PARALLEL_SUBDIRECTORIES:=.distclean.subdirectories)
  internal-master-subdirectories-check: $(PARALLEL_SUBDIRECTORIES:=.check.subdirectories)
  internal-master-subdirectories-strings: $(PARALLEL_SUBDIRECTORIES:=.strings.subdirectories)

  # See Master/rules.make as to why we use .PRECIOUS instead of .PHONY
  # here.
  .PRECIOUS: %.subdirectories

  %.subdirectories:
	$(ECHO_NOTHING_RECURSIVE_MAKE)directory=$(basename $*); \
          operation=$(subst .,,$(suffix $*)); \
	  abs_build_dir="$(ABS_GNUSTEP_BUILD_DIR)"; \
	  $(INSIDE_ECHO_MAKING_OPERATION_IN_DIRECTORY) \
	  if [ "$${abs_build_dir}" = "." ]; then \
	    gsbuild="."; \
	  else \
	    gsbuild="$${abs_build_dir}/$$instance"; \
	  fi; \
	  if $(MAKE) -C $$directory -f $(MAKEFILE_NAME) $(GNUSTEP_MAKE_NO_PRINT_DIRECTORY_FLAG) --no-keep-going \
	       $$operation \
	       GNUSTEP_BUILD_DIR="$$gsbuild" _GNUSTEP_MAKE_PARALLEL=no; then \
	    :; else exit $$?; \
	  fi$(END_ECHO_RECURSIVE_MAKE)

  # We still do 'install' and 'uninstall' in non-parallel mode, to
  # prevent any race conditions with the creation of installation
  # directories.  TODO: It would be cool to make this configurable 
  # so you could make it parallel if you so wish.
  internal-install internal-uninstall::
	$(ECHO_NOTHING_RECURSIVE_MAKE)operation=$(subst internal-,,$@); \
	  abs_build_dir="$(ABS_GNUSTEP_BUILD_DIR)"; \
	for directory in $(PARALLEL_SUBDIRECTORIES); do \
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