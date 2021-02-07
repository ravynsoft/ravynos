#   -*-makefile-*-
#   Master/tool.make
#
#   Master Makefile rules to build GNUstep-based command line tools.
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Nicola Pero <nicola@brainstorm.co.uk>
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

TOOL_NAME := $(strip $(TOOL_NAME))

# We need to create/delete the GNUSTEP_BUILD_DIR/Resources directory
# in the Master invocation stage, to prevent different tools, built in
# parallel, from trying to create it concurrently and causing race
# conditions.  But, if no tool has a resource bundle, we don't create
# or delete the directory at all.  ;-)
TOOLS_WITH_RESOURCE_BUNDLES = $(strip $(foreach tool,$(TOOL_NAME),$($(tool)_HAS_RESOURCE_BUNDLE:yes=$(tool))))

ifneq ($(TOOLS_WITH_RESOURCE_BUNDLES),)
MAYBE_GNUSTEP_BUILD_DIR_RESOURCES = $(GNUSTEP_BUILD_DIR)/Resources
$(GNUSTEP_BUILD_DIR)/Resources:
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

# On distclean, we want to efficiently wipe out the Resources/
# directory.
internal-clean::
	rm -rf $(MAYBE_GNUSTEP_BUILD_DIR_RESOURCES)/$(TOOL_NAME)
else
MAYBE_GNUSTEP_BUILD_DIR_RESOURCES =
endif


ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

# Standard building
internal-all:: $(GNUSTEP_OBJ_DIR) $(MAYBE_GNUSTEP_BUILD_DIR_RESOURCES) $(TOOL_NAME:=.all.tool.variables)

else

# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yes.
# That sub-make invocation will fire off the building of the tools in
# parallel.  This is great as the entire building (including the
# linking) of the tools is then parallelized.

# Please note that we need to create the ./obj directory (and the
# GNUSTEP_BUILD_DIR/Resources directory if any tool has a resource
# bundle) before we fire off all the parallel sub-makes, else they'll
# be a race condition to create it. (typically what happens is that
# two sub-makes detect that it needs creating, the first one creates
# it, and when the second one tries to create it, it will fail as it's
# already been created).
internal-all:: $(GNUSTEP_OBJ_DIR) $(MAYBE_GNUSTEP_BUILD_DIR_RESOURCES)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-tool-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-tool-all

internal-master-tool-all: $(TOOL_NAME:=.all.tool.variables)

endif

# TODO: Installing and uninstalling in parallel would be extremely
# cool, but if you fire off many sub-makes (one for each instance) in
# parallel, you end up with a lot of race conditions as the tools are
# most often installed in the same directories, which the different
# sub-makes will attempt to create concurrently.  A better solution
# would be to fire off a single Master invocation with
# _GNUSTEP_MAKE_PARELLEL enabled, and in there install all the tools
# using parallel rules.  This requires moving all the tool installation 
# code from Instance/ to Master/.
internal-install:: $(TOOL_NAME:=.install.tool.variables)

internal-uninstall:: $(TOOL_NAME:=.uninstall.tool.variables)

internal-clean::

internal-distclean::

TOOLS_WITH_SUBPROJECTS = $(strip $(foreach tool,$(TOOL_NAME),$(patsubst %,$(tool),$($(tool)_SUBPROJECTS))))
ifneq ($(TOOLS_WITH_SUBPROJECTS),)
internal-clean:: $(TOOLS_WITH_SUBPROJECTS:=.clean.tool.subprojects)
internal-distclean:: $(TOOLS_WITH_SUBPROJECTS:=.distclean.tool.subprojects)
endif

# TODO: It should be really safe to parallelize the 'strings' targets,
# but it's worth checking to make sure we're not breaking anything.
internal-strings:: $(TOOL_NAME:=.strings.tool.variables)

$(TOOL_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going $@.all.tool.variables$(END_ECHO_RECURSIVE_MAKE)
